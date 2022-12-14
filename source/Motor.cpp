#include "Motor.h"

namespace spirit {

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

void Motor::velocity(const float rps)
{
    _velocity = rps;
}

float Motor::get_velocity() const
{
    return _velocity;
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
    switch (type) {
        case Decay::Slow:
        case Decay::Fast:
            break;

        // 非対応
        case Decay::Mixed:
        default:
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
