#include "spirit/include/Motor.h"

#include "spirit/include/Error.h"

namespace spirit {

void Motor::control_system(const ControlSystem type)
{
    switch (type) {
        case ControlSystem::PWM:
        case ControlSystem::Speed:
            break;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor control system (%d)", static_cast<uint32_t>(type));
            return;
    }

    _control_system = type;
}

Motor::ControlSystem Motor::get_control_system() const
{
    return _control_system;
}

void Motor::duty_cycle(const float value)
{
    if (1.00F < value) {
        _duty_cycle = 1.00F;

        Error::get_instance().warning(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                      "Duty cycle (%g) is greater than 1.00, so it will be 1.00", value);
    } else if (value < 0.00F) {
        _duty_cycle = 0.00F;

        Error::get_instance().warning(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                      "Duty cycle (%g) is less than 0.00, so it will be 0.00", value);
    } else {
        _duty_cycle = value;
    }
}

float Motor::get_duty_cycle() const
{
    return _duty_cycle;
}

void Motor::speed(const float rps)
{
    /// @todo rpsはマイナスにならないはずなので、範囲チェックを足す
    _speed = rps;
}

float Motor::get_speed() const
{
    return _speed;
}

void Motor::pid_gain_factor(const float Kp, const float Ki, const float Kd)
{
    /// @todo Kp, Ki, Kdの範囲チェック

    _Kp = Kp;
    _Ki = Ki;
    _Kd = Kd;
}

void Motor::get_pid_gain_factor(float& Kp, float& Ki, float& Kd) const
{
    Kp = _Kp;
    Ki = _Ki;
    Kd = _Kd;
}

void Motor::state(const State type)
{
    switch (type) {
        case State::Coast:
        case State::CW:
        case State::CCW:
        case State::Brake:
            break;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor state (%d)", static_cast<uint32_t>(type));
            return;
    }

    _state = type;
}

Motor::State Motor::get_state() const
{
    return _state;
}

void Motor::change_level(const ChangeLevelTarget target, const ChangeLevel level)
{
    switch (level) {
        case ChangeLevel::OFF:
        case ChangeLevel::Low:
        case ChangeLevel::Middle:
        case ChangeLevel::High:
        case ChangeLevel::Max:
            break;
        case ChangeLevel::Manual:
            Error::get_instance().error(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                        "Invalid motor change level (ChangeLevel::Manual)");
            return;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor change level (%d)", static_cast<uint32_t>(level));
            return;
    }

    switch (target) {
        case ChangeLevelTarget::Rise:
            _rise_change_level = level;
            break;
        case ChangeLevelTarget::Fall:
            _fall_change_level = level;
            break;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor change level target (%d)", static_cast<uint32_t>(target));
            return;
    }
}

void Motor::change_level(const ChangeLevelTarget target, const float duty_cycle)
{
    /// @todo duty_cycleは最大でも200%しか変化しないはずなので、範囲チェックをする

    if (duty_cycle < minimum_maximum_change_duty_cycle) {
        Error::get_instance().error(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                    "Duty cycle (%g) is less than minimum duty cycle (%g)",
                                    minimum_maximum_change_duty_cycle);
        return;
    }

    switch (target) {
        case ChangeLevelTarget::Rise:
            _rise_change_level              = ChangeLevel::Manual;
            _rise_maximum_change_duty_cycle = duty_cycle;
            break;
        case ChangeLevelTarget::Fall:
            _fall_change_level              = ChangeLevel::Manual;
            _fall_maximum_change_duty_cycle = duty_cycle;
            break;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor change level target (%d)", static_cast<uint32_t>(target));
            return;
    }
}

Motor::ChangeLevel Motor::get_change_level(const ChangeLevelTarget target) const
{
    switch (target) {
        case ChangeLevelTarget::Rise:
            return _rise_change_level;
        case ChangeLevelTarget::Fall:
            return _fall_change_level;

        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor change level target (%d)", static_cast<uint32_t>(target));
            return ChangeLevel::OFF;
    }
}

float Motor::get_maximum_change_duty_cycle(ChangeLevelTarget target) const
{
    ChangeLevel level = get_change_level(target);

    /// ChangeLevel::Manual 時に最大変化デューティ比を返す
    auto manual_maximum_change_duty_cycle = [this](ChangeLevelTarget target) {
        switch (target) {
            case ChangeLevelTarget::Rise:
                return _rise_maximum_change_duty_cycle;
            case ChangeLevelTarget::Fall:
                return _fall_maximum_change_duty_cycle;

                // default に来ることは、get_change_level で既にチェックしているので通常の利用ではありえないため、カバレッジ計測から除外する
                // LCOV_EXCL_START
            default:
                Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                            "Unknown motor change level target (%d)", static_cast<uint32_t>(target));
                return 0.00F;
                // LCOV_EXCL_STOP
        }
    };

    switch (level) {
        case ChangeLevel::Manual:
            return manual_maximum_change_duty_cycle(target);
        case ChangeLevel::OFF:
            return 1.00F;
        case ChangeLevel::Low:
            return 0.001F;
        case ChangeLevel::Middle:
            return 0.0001F;
        case ChangeLevel::High:
            return 0.0001F;
        case ChangeLevel::Max:
            return 0.00001F;

            // default に来ることは、change_level で既にチェックしているので通常の利用ではありえないため、カバレッジ計測から除外する
            // LCOV_EXCL_START
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor change level (%d)", static_cast<uint32_t>(level));
            return 0.00F;
            // LCOV_EXCL_STOP
    }
}

void Motor::pulse_period(const float seconds)
{
    if (max_pulse_period < seconds) {
        _pulse_period = max_pulse_period;

        Error::get_instance().warning(
            Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
            "Pulse period (%g) is greater than max pulse period (%g), so it will be max pulse period (%g)", seconds,
            max_pulse_period, max_pulse_period);
    } else if (seconds < min_pulse_period) {
        _pulse_period = min_pulse_period;

        Error::get_instance().warning(
            Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
            "Pulse period (%g) is less than min pulse period (%g), so it will be min pulse period (%g)", seconds,
            min_pulse_period, min_pulse_period);
    } else {
        _pulse_period = seconds;
    }
}

float Motor::get_pulse_period() const
{
    return _pulse_period;
}

void Motor::release_time(const float seconds)
{
    /// @todo マイナス秒はありえないので、エラーにする
    _release_time = seconds;
}

float Motor::get_release_time() const
{
    return _release_time;
}

void Motor::decay(const Decay type)
{
    switch (type) {
        case Decay::Slow:
        case Decay::Fast:
            break;
        case Decay::Mixed:
            Error::get_instance().error(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                        "Invalid motor decay (Decay::Mixed)");
            return;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor decay (%d)", static_cast<uint32_t>(type));
            return;
    }

    _decay = type;
}

Motor::Decay Motor::get_decay() const
{
    return _decay;
}

void Motor::pwm_side(const PwmSide type)
{
    switch (type) {
        case PwmSide::Low:
        case PwmSide::High:
            break;
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor PWM side (%d)", static_cast<uint32_t>(type));
            return;
    }

    _pwm_side = type;
}

Motor::PwmSide Motor::get_pwm_side() const
{
    return _pwm_side;
}

void Motor::reset(const bool enabled)
{
    _reset = enabled;
}

bool Motor::get_reset() const
{
    return _reset;
}

void Motor::sleep(const bool enabled)
{
    _sleep = enabled;
}

bool Motor::get_sleep() const
{
    return _sleep;
}

}  // namespace spirit
