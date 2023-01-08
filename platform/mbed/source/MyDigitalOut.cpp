#include "../include/MyDigitalOut.h"

#include "mbed.h"

namespace spirit {

MyDigitalOut::MyDigitalOut(PinName pin) : _digital_out(pin)
{
}

MyDigitalOut::~MyDigitalOut()
{
}

void MyDigitalOut::write(const uint32_t value)
{
    _digital_out.write(value);
}

uint32_t MyDigitalOut::read()
{
    int value = _digital_out.read();
    return value;
}

}  // namespace spirit
