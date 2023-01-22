#include "Motor.h"

#include "Error.h"

namespace spirit {

void Motor::control_system(const ControlSystem type)
{
    switch (type) {
        case ControlSystem::PWM:
        case ControlSystem::Speed:
            break;
        default:
            Error&      error = Error::get_instance();
            std::string message   = "Unkown motor type (" + std::to_string(static_cast<uint32_t>(type)) + ")";
            error.error(Error::Type::UnknownValue, 0, message.c_str(), __FILE__, __func__, __LINE__);
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
    } else if (value < 0.00F) {
        _duty_cycle = 0.00F;
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
            Error&      error = Error::get_instance();
            std::string message   = "Unkown motor state: " + std::to_string(static_cast<uint32_t>(type));
            error.error(Error::Type::UnknownValue, 0, message.c_str(), __FILE__, __func__, __LINE__);
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
    switch (target) {
        case ChangeLevelTarget::Rise:
            _rise_change_level = level;
            break;
        case ChangeLevelTarget::Fall:
            _fall_change_level = level;
            break;
        default:
            Error&      error = Error::get_instance();
            std::string message =
                "Unkown motor change level target (" + std::to_string(static_cast<uint32_t>(target)) + ")";
            error.error(Error::Type::UnknownValue, 0, message.c_str(), __FILE__, __func__, __LINE__);
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
            return ChangeLevel::OFF;
    }
}

void Motor::pulse_period(const float seconds)
{
    if (max_pulse_period < seconds) {
        _pulse_period = max_pulse_period;
    } else if (seconds < min_pulse_period) {
        _pulse_period = min_pulse_period;
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
    Error& error = Error::get_instance();
    switch (type) {
        case Decay::Slow:
        case Decay::Fast:
            break;
        case Decay::Mixed:
            error.error(Error::Type::InvalidValue, 0, "Invalid motor decay (Decay::Mixed)", __FILE__, __func__,
                        __LINE__);
            return;
        default:
            std::string message = "Unkown motor decay (" + std::to_string(static_cast<uint32_t>(type)) + ")";
            error.error(Error::Type::UnknownValue, 0, message.c_str(), __FILE__, __func__, __LINE__);
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
            Error&      error = Error::get_instance();
            std::string message   = "Unkown motor PWM side (" + std::to_string(static_cast<uint32_t>(type)) + ")";
            error.error(Error::Type::UnknownValue, 0, message.c_str(), __FILE__, __func__, __LINE__);
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
