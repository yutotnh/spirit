#if defined(__MBED__) && !defined(ARDUINO)
// ArduinoのMbedではCallbackが使えないので、定義しない

#include "spirit/platform/mbed/include/InterruptIn.h"

#include "mbed.h"

namespace spirit {

namespace mbed {

InterruptIn::InterruptIn(PinName pin) : _interrupt_in(pin)
{
}

InterruptIn::~InterruptIn()
{
}

void InterruptIn::rise(Callback<void()> func_rise)
{
    _interrupt_in.rise(func_rise);
}

void InterruptIn::fall(Callback<void()> func_fall)
{
    _interrupt_in.fall(func_fall);
}

uint32_t InterruptIn::read()
{
    return _interrupt_in.read();
}

}  // namespace mbed

}  // namespace spirit

#endif  // defined(__MBED__) && !defined(ARDUINO)
