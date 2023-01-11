#include "platform/mbed/include/PwmOut.h"

#include "mbed.h"

namespace spirit {

namespace mbed {

PwmOut::PwmOut(PinName pin) : _pwm_out(pin)
{
}

PwmOut::~PwmOut()
{
}

void PwmOut::write(const float value)
{
    _pwm_out.write(value);
}

float PwmOut::read()
{
    return _pwm_out.read();
}

void PwmOut::period(const float seconds)
{
    _pwm_out.period(seconds);
}

}  // namespace mbed

}  // namespace spirit
