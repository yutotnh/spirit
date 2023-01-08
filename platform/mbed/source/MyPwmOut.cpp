#include "../include/MyPwmOut.h"

#include "mbed.h"
namespace spirit {

MyPwmOut::MyPwmOut(PinName pin) : _pwm_out(pin)
{
}

MyPwmOut::~MyPwmOut()
{
}

void MyPwmOut::write(const float value)
{
    _pwm_out.write(value);
}

float MyPwmOut::read()
{
    return _pwm_out.read();
}

void MyPwmOut::period(const float seconds)
{
    _pwm_out.period(seconds);
}

};  // namespace spirit
