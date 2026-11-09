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
    _interrupt_in.rise(::mbed::Callback<void()>());
    _interrupt_in.fall(::mbed::Callback<void()>());
}

void InterruptIn::rise(InterruptHandler* handler)
{
    if (handler == nullptr) {
        _interrupt_in.rise(::mbed::Callback<void()>());
    } else {
        _interrupt_in.rise(::mbed::callback(handler, &InterruptHandler::on_interrupt));
    }
}

void InterruptIn::fall(InterruptHandler* handler)
{
    if (handler == nullptr) {
        _interrupt_in.fall(::mbed::Callback<void()>());
    } else {
        _interrupt_in.fall(::mbed::callback(handler, &InterruptHandler::on_interrupt));
    }
}

uint32_t InterruptIn::read()
{
    return _interrupt_in.read();
}

}  // namespace mbed

}  // namespace spirit

#endif  // defined(__MBED__) && !defined(ARDUINO)
