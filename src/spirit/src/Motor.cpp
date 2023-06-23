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
            constexpr char message_format[] = "Unknown motor control system (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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

        constexpr char message_format[] = "Duty cycle (%1.4e) is greater than 1.00, so it will be 1.00";
        char           message[sizeof(message_format) + Error::max_float_1_4_e_length];
        snprintf(message, sizeof(message), message_format, value);
        Error::get_instance().warning(Error::Type::InvalidValue, 0, message, __FILE__, __func__, __LINE__);
    } else if (value < 0.00F) {
        _duty_cycle = 0.00F;

        constexpr char message_format[] = "Duty cycle (%1.4e) is less than 0.00, so it will be 0.00";
        char           message[sizeof(message_format) + Error::max_float_1_4_e_length];
        snprintf(message, sizeof(message), message_format, value);
        Error::get_instance().warning(Error::Type::InvalidValue, 0, message, __FILE__, __func__, __LINE__);
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
    _speed = rps;
}

float Motor::get_speed() const
{
    return _speed;
}

void Motor::pid_gain_factor(const float Kp, const float Ki, const float Kd)
{
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
            constexpr char message_format[] = "Unknown motor state (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
            Error::get_instance().error(Error::Type::InvalidValue, 0,
                                        "Invalid motor change level (ChangeLevel::Manual)", __FILE__, __func__,
                                        __LINE__);
            return;
        default:
            constexpr char message_format[] = "Unknown motor change level (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(level));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
            constexpr char message_format[] = "Unknown motor change level target (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(target));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            return;
    }
}

void Motor::change_level(const ChangeLevelTarget target, const float duty_cycle)
{
    if (duty_cycle < minimum_maximum_change_duty_cycle) {
        constexpr char message_format[] = "Duty cycle (%1.4e) is less than minimum duty cycle (%1.4e)";
        char           message[sizeof(message_format) + Error::max_float_1_4_e_length * 2];
        snprintf(message, sizeof(message), message_format, duty_cycle, minimum_maximum_change_duty_cycle);
        Error::get_instance().error(Error::Type::InvalidValue, 0, message, __FILE__, __func__, __LINE__);
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
            constexpr char message_format[] = "Unknown motor change level target (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(target));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
            constexpr char message_format[] = "Unknown motor change level target (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(target));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
                constexpr char message_format[] = "Unknown motor change level target (%d)";
                char           message[sizeof(message_format) + Error::max_uint32_t_length];
                snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(target));
                Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
            constexpr char message_format[] = "Unknown motor change level (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(level));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            return 0.00F;
            // LCOV_EXCL_STOP
    }
}

void Motor::pulse_period(const float seconds)
{
    if (max_pulse_period < seconds) {
        _pulse_period = max_pulse_period;

        constexpr char message_format[] =
            "Pulse period (%1.4e) is greater than max pulse period (%1.4e), so it will be max pulse period (%1.4e)";
        char message[sizeof(message_format) + Error::max_float_1_4_e_length * 3];
        snprintf(message, sizeof(message), message_format, seconds, max_pulse_period, max_pulse_period);
        Error::get_instance().warning(Error::Type::InvalidValue, 0, message, __FILE__, __func__, __LINE__);
    } else if (seconds < min_pulse_period) {
        _pulse_period = min_pulse_period;

        constexpr char message_format[] =
            "Pulse period (%1.4e) is less than min pulse period (%1.4e), so it will be min pulse period (%1.4e)";
        char message[sizeof(message_format) + Error::max_float_1_4_e_length * 3];
        snprintf(message, sizeof(message), message_format, seconds, min_pulse_period, min_pulse_period);
        Error::get_instance().warning(Error::Type::InvalidValue, 0, message, __FILE__, __func__, __LINE__);
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
            Error::get_instance().error(Error::Type::InvalidValue, 0, "Invalid motor decay (Decay::Mixed)", __FILE__,
                                        __func__, __LINE__);
            return;
        default:
            constexpr char message_format[] = "Unknown motor decay (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
            constexpr char message_format[] = "Unknown motor PWM side (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
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
