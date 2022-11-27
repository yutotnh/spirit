#include "A3921.h"

namespace spirit {

A3921::A3921(InterfaceDigitalOut& sr, InterfacePwmOut& pwmh, InterfacePwmOut& pwml, InterfacePwmOut& phase,
             InterfaceDigitalOut& reset)
    : _sr(sr), _pwmh(pwmh), _pwml(pwml), _phase(phase), _reset(reset)
{
    sleep(false);
    _pwmh.period(InterfaceMotor::default_pulse_period);
    _pwml.period(InterfaceMotor::default_pulse_period);
    _phase.period(InterfaceMotor::default_pulse_period);
    run();
}

void A3921::sleep(const bool enabled)
{
    if (enabled) {
        _reset.write(0U);
    } else {
        _reset.write(1U);
    }
}

void A3921::reset()
{
    _reset.write(1);
    // TODO time時間待つ (0.1us < time < 3.5us)
    _reset.write(0);
}

void A3921::duty_cycle(const float value)
{
    _duty_cycle = value;
}

void A3921::state(const State type)
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

void A3921::decay(const Decay type)
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

void A3921::run()
{
    switch (_decay) {
        case Decay::Slow:
            run_slow_decay();
            break;

        case Decay::Fast:
            run_fast_decay();
            break;

        case Decay::Mixed:
        default:
            return;
    }
}

void A3921::run_slow_decay()
{
    switch (_state) {
        case State::Coast:
            _sr.write(0);
            _pwmh.write(0.00F);
            _pwml.write(0.00F);
            _phase.write(0.00F);
            break;

        case State::CW:
            // a to b
            _sr.write(1);
            _pwmh.write(1.00F);
            _pwml.write(_duty_cycle);
            _phase.write(1.00F);
            break;

        case State::CCW:
            // b to a
            _sr.write(1);
            _pwmh.write(1.00F);
            _pwml.write(_duty_cycle);
            _phase.write(0.00F);
            break;

        case State::Brake:
            _sr.write(0);
            _pwmh.write(0.00F);
            _pwml.write(1.00F);
            _phase.write(0.00F);
            break;

        default:
            break;
    }
}

void A3921::run_fast_decay()
{
    switch (_state) {
        case State::Coast:
            _sr.write(0);
            _pwmh.write(0.00F);
            _pwml.write(0.00F);
            _phase.write(0.00F);
            break;

        case State::CW:
            // a to b
            _sr.write(0);
            _pwmh.write(1.00F);
            _pwml.write(1.00F);
            _phase.write(0.50F + (_duty_cycle / 2.0F));
            break;

        case State::CCW:
            // b to a
            _sr.write(0);
            _pwmh.write(1.00F);
            _pwml.write(1.00F);
            _phase.write(0.50F - (_duty_cycle / 2.0F));
            break;

        case State::Brake:
            _sr.write(0);
            _pwmh.write(0.00F);
            _pwml.write(1.00F);
            _phase.write(0.00F);
            break;

        default:
            break;
    }
}

void A3921::pulse_period(const float seconds)
{
    if ((InterfaceMotor::min_pulse_period <= seconds) && (seconds <= InterfaceMotor::max_pulse_period)) {
        _pwmh.period(seconds);
        _pwml.period(seconds);
        _phase.period(seconds);
    }
}

}  // namespace spirit
