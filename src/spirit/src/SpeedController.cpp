#include "spirit/include/SpeedController.h"

#include <float.h>

#include "spirit/include/Error.h"

namespace spirit {

SpeedController::SpeedController(InterfaceInterruptIn& a_phase, InterfaceInterruptIn& b_phase,
                                 const int pulses_per_revolution)
    : _a_phase(a_phase), _b_phase(b_phase)
{
    if (pulses_per_revolution <= 0) {
        Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                    "[pulses_per_revolution = %d] value must be greater than 0", pulses_per_revolution);
    } else {
        _deg_unit = 360.0f / static_cast<float>(pulses_per_revolution) / 4.0f;
    }

#if defined(__MBED__) && !defined(ARDUINO)
    _a_phase.rise(mbed::callback(this, &SpeedController::a_phase_interrupt));
    _a_phase.fall(mbed::callback(this, &SpeedController::a_phase_interrupt));
    _b_phase.rise(mbed::callback(this, &SpeedController::b_phase_interrupt));
    _b_phase.fall(mbed::callback(this, &SpeedController::b_phase_interrupt));
#elif defined(ARDUINO)
    // Arduino の attachInterrupt() は呼び出し先にインスタンスを渡せないので、ここでは登録しない
    // 呼び出し側が A相・B相の割り込みハンドラから a_phase_interrupt() / b_phase_interrupt() を呼ぶ
#else
    std::function<void(void)> func_a = std::bind(&SpeedController::a_phase_interrupt, this);
    std::function<void(void)> func_b = std::bind(&SpeedController::b_phase_interrupt, this);
    _a_phase.rise(func_a);
    _a_phase.fall(func_a);
    _b_phase.rise(func_b);
    _b_phase.fall(func_b);
#endif
    reset();
}

float SpeedController::calculation(float target_rps, float dt)
{
    // NaN も弾くため、否定の形で比較する
    if (!(0.0f <= target_rps)) {
        Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                    "[target_rps = %g] value cannot be negative or NaN", target_rps);
        return 0.0f;
    }

    if (!(0.0f < dt)) {
        Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                    "[dt = %g] value cannot be less than 0", dt);
        return 0.0f;
    }

    _rps = rps_calculation(dt);

    float error = target_rps - _rps;

    // 積分は誤差を時間で積分する(制御周期 dt が変動しても正しく積分できる)
    _sum_error += error * dt;

    // 積分項の出力が上限・下限に達している間は、誤差を蓄積し続けない(積分の巻き上がり防止)
    if (_ki > 0.0f) {
        const float max_sum_error = _high_limit / _ki;
        const float min_sum_error = _low_limit / _ki;
        if (_sum_error > max_sum_error) {
            _sum_error = max_sum_error;
        } else if (_sum_error < min_sum_error) {
            _sum_error = min_sum_error;
        }
    }

    // 微分は、目標値の変化で出力が跳ね上がらないように、誤差ではなく測定値(rps)に対して行う
    // (目標値が一定なら、誤差の微分の符号を反転したものと一致する)
    float p = _kp * error;
    float i = limiter(_ki * _sum_error);
    float d = -_kd * (_rps - _last_rps) / dt;

    _last_rps = _rps;

    return limiter(p + i + d);
}

float SpeedController::rps_calculation(float dt)
{
    if (!(0.0f < dt)) {
        Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                    "[dt = %g] value cannot be less than 0", dt);
        return 0.0f;
    }

    float rps;
    _angle_buff_index++;

    if (_angle_buff_index == _angle_buff_max) {
        _angle_buff_index = 0;
        first_loop        = false;
    } else if (first_loop) {
        // 最初の一周の間も角度を記録しておく
        // (記録しないと、二周目の差分がリセット後の総回転量になり、rps が実際より大きく出る)
        _angle_buff[_angle_buff_index] = _angle_counter.load(std::memory_order_relaxed);

        rps = angle() / 360.0f / (dt * _angle_buff_index);
        if (rps < 0.0f) {
            rps *= -1;
        }
        return rps;
    }

    const int angle_counter        = _angle_counter.load(std::memory_order_relaxed);
    int       angle_diff           = angle_counter - _angle_buff[_angle_buff_index];
    _angle_buff[_angle_buff_index] = angle_counter;

    rps = (angle_diff * _deg_unit) / 360.0f / (dt * _angle_buff_max);
    if (rps < 0.0f) {
        rps *= -1;
    }
    return rps;
}

bool SpeedController::limit(float high_limit, float low_limit)
{
    if (high_limit > low_limit) {
        _high_limit = high_limit;
        _low_limit  = low_limit;
    } else {
        Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                    "[high_limit = %g, low_limit = %g]The relationship between the upper limit value "
                                    "and the lower limit value is strange",
                                    high_limit, low_limit);
        return false;
    }

    return true;
}

bool SpeedController::pid_gain(float kp, float ki, float kd)
{
    if ((_kp == kp) && (_ki == ki) && (_kd == kd)) {
        return false;
    }

    _kp = kp;
    _ki = ki;
    _kd = kd;

    return true;
}

void SpeedController::reset()
{
    _angle_counter.store(0, std::memory_order_relaxed);
    _angle_buff_index = 0;
    _sum_error        = 0.0f;
    _last_rps         = 0.0f;
    _rps              = 0.0f;
    for (int i = 0; i < _angle_buff_max; i++) {
        _angle_buff[i] = 0;
    }
    first_loop = true;
}

float SpeedController::limiter(float val)
{
    if (val > _high_limit) {
        return _high_limit;
    } else if (val < _low_limit) {
        return _low_limit;
    }
    return val;
}

void SpeedController::add_angle_counter(const int delta)
{
    // 書き込むのは割り込みだけなので、ロックのない load と store で足せる (fetch_add は、コアによっては追加のライブラリが必要になる)
    _angle_counter.store(_angle_counter.load(std::memory_order_relaxed) + delta, std::memory_order_relaxed);
}

void SpeedController::a_phase_interrupt()
{
    if (_a_phase.read() != _b_phase.read()) {
        add_angle_counter(1);
    } else {
        add_angle_counter(-1);
    }
}

void SpeedController::b_phase_interrupt()
{
    if (_a_phase.read() == _b_phase.read()) {
        add_angle_counter(1);
    } else {
        add_angle_counter(-1);
    }
}

float SpeedController::angle()
{
    return static_cast<float>(_angle_counter.load(std::memory_order_relaxed)) * _deg_unit;
}

float SpeedController::rps()
{
    return _rps;
}

// C++14 では、 static constexpr メンバーを参照渡しなどで使う(odr-use)と、クラス外の定義が必要になる (C++17 以降は不要)
#if __cplusplus < 201703L
constexpr int SpeedController::default_pulses_per_revolution;
#endif

}  // namespace spirit
