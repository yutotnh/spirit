#ifndef SPIRIT_MOTOR_H
#define SPIRIT_MOTOR_H

#include <cstdint>

namespace spirit {

/** @brief モーターをあらわすクラス
 */
class Motor {
public:
    /**
     * @enum State
     * @brief モーターの回転方向
     */
    enum class State {
        /// 空転(緩やかに減速)
        Coast,
        /// 時計回り(Clock Wise)
        CW,
        /// 反時計回り(Counter Clock Wise)
        CCW,
        /// ブレーキ
        Brake,
    };

    enum class ChangeLevelTarget {
        Rise,
        Fall,
    };

    enum class ChangeLevel {
        /// 指定したデューティーを直ちにモーターに出力する
        OFF,
        /// 指定したデューティーまでほんの少し余裕を持たせてモーターに出力する
        Low,
        /// 指定したデューティーまで少し余裕を持たせてモーターに出力する
        Middle,
        /// 指定したデューティーまで余裕を持たせてモーターに出力する
        High,
        /// 指定したデューティーまでかなり余裕を持たせてモーターに出力する
        Max
    };

    /**
     * @enum Decay
     * @brief モーターの制御方法を設定するための値
     */
    enum class Decay {
        /// Slow decay
        Slow,
        /// Mixed decay(未対応)
        Mixed,
        /// Fast decay
        Fast,
    };

    /**
     * @enum PwmSide
     * @brief Slow decayのPWM sideを設定するための値
     */
    enum class PwmSide : uint8_t {
        /// Low side
        Low,
        /// High side
        High,
    };

    void  duty_cycle(float value);
    float get_duty_cycle() const;

    void  velocity(float rps);
    float get_velocity() const;

    void  state(State type);
    State get_state() const;

    void        change_level(ChangeLevelTarget target, ChangeLevel level);
    ChangeLevel get_change_level(ChangeLevelTarget target) const;

    void  pulse_period(float seconds);
    float get_pulse_period() const;

    void  release_time(float seconds);
    float get_release_time() const;

    void  decay(Decay type);
    Decay get_decay() const;

    void    pwm_side(PwmSide type);
    PwmSide pwm_side() const;

    void reset(bool enable);
    bool get_reset() const;

    void sleep(bool enable);
    bool get_sleep() const;

    static constexpr State       default_state             = State::Brake;
    static constexpr ChangeLevel default_rise_change_level = ChangeLevel::OFF;
    static constexpr ChangeLevel default_fall_change_level = ChangeLevel::OFF;
    static constexpr float       default_pulse_period      = 1.0F / 5'000.0F;
    static constexpr float       min_pulse_period          = 1.0F / 60'000.0F;
    static constexpr float       max_pulse_period          = 1.0F / 10.0F;
    static constexpr Decay       default_decay             = Decay::Slow;
    static constexpr PwmSide     default_pwm_side          = PwmSide::Low;
    static constexpr bool        default_sleep             = false;

private:
    float       _duty_cycle{0.0F};
    float       _velocity{0.0F};
    State       _state{default_state};
    ChangeLevel _rise_change_level{default_rise_change_level};
    ChangeLevel _fall_change_level{default_fall_change_level};
    float       _pulse_period{default_pulse_period};
    float       _release_time{0.0F};
    Decay       _decay{default_decay};
    PwmSide     _pwm_side{default_pwm_side};
    bool        _reset{false};
    bool        _sleep{default_sleep};
};

}  // namespace spirit

#endif  // SPIRIT_MOTOR_H
