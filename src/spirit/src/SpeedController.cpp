#include "spirit/include/SpeedController.h"

namespace spirit {

SpeedController::SpeedController(InterfaceInterruptIn& a_phase, InterfaceInterruptIn& b_phase)
    : _a_phase(a_phase), _b_phase(b_phase)
{
#if defined(__MBED__) && !defined(ARDUINO)
    _a_phase.rise(mbed::callback(this, &SpeedController::a_phase_interrupt));
    _a_phase.fall(mbed::callback(this, &SpeedController::a_phase_interrupt));
    _b_phase.rise(mbed::callback(this, &SpeedController::b_phase_interrupt));
    _b_phase.fall(mbed::callback(this, &SpeedController::b_phase_interrupt));
#elif defined(ARDUINO)
    // TODO Arduinoに対応
#else
    std::function<void(void)> func_a = std::bind(&SpeedController::a_phase_interrupt, this);
    std::function<void(void)> func_b = std::bind(&SpeedController::b_phase_interrupt, this);
    _a_phase.rise(func_a);
    _a_phase.fall(func_a);
    _b_phase.rise(func_b);
    _b_phase.fall(func_b);
#endif
}

float SpeedController::calculation(float target_rps, float dt)
{
    float error = target_rps - rps(dt);
    _sum_error += error;
    _delta_error = error - _last_error;

    float p = _kp * error;
    float i = limiter(_ki * _sum_error * dt / 2.0f);
    float d = _kd * _delta_error / dt;

    _last_error = error;

    return limiter(p + i + d);
}

float SpeedController::rps(float dt)
{
    float angle_diff;

    _angle_buff_index++;

    if (_angle_buff_index == _angle_buff_max) {
        _angle_buff_index              = 0;
        _angle_buff[_angle_buff_index] = _angle_counter;
        angle_diff                     = _angle_buff[_angle_buff_index] - _angle_buff[_angle_buff_max - 1];
    } else {
        _angle_buff[_angle_buff_index] = _angle_counter;
        angle_diff                     = _angle_buff[_angle_buff_index] - _angle_buff[_angle_buff_index - 1];
    }

    /* or
    angle_buff[angle_buff_index] = angle_counter;

    if(!angle_buff_index){
        angle_diff = angle_buff[angle_buff_index] - angle_buff[angle_buff_max   - 1];
    } else {
        angle_diff = angle_buff[angle_buff_index] - angle_buff[angle_buff_index - 1];
    }

    if(angle_buff_index++ == angle_buff_max){
        angle_buff_index = 0;
    }
    */

    return angle_diff * _deg_unit / 360.0f / dt;
}

void SpeedController::limit(float high_limit, float low_limit)
{
    if (high_limit > low_limit) {
        _high_limit = high_limit;
        _low_limit  = low_limit;
    } else {
        _high_limit = low_limit;
        _low_limit  = high_limit;
    }
}

bool SpeedController::pid_gain(float kp, float ki, float kd)
{
    if (_kp == kp) {
        if (_ki == ki) {
            if (_kd == kd) {
                return false;
            }
        }
    }

    _kp = kp;
    _ki = ki;
    _kd = kd;

    return true;
}

void SpeedController::reset()
{
    _angle_counter    = 0;
    _angle_buff[0]    = 0;
    _angle_buff_index = 0;
    _sum_error        = 0.0f;
    _last_error       = 0.0f;
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

void SpeedController::a_phase_interrupt()
{
    if (_a_phase.read() != _b_phase.read()) {
        _angle_counter++;
    } else {
        _angle_counter--;
    }
}

void SpeedController::b_phase_interrupt()
{
    if (_a_phase.read() == _b_phase.read()) {
        _angle_counter++;
    } else {
        _angle_counter--;
    }
}

float SpeedController::angle()
{
    return _angle_counter * _deg_unit;
}

}  // namespace spirit
