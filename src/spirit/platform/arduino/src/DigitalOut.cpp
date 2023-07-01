#ifdef ARDUINO

#include "spirit/platform/arduino/include/DigitalOut.h"

namespace spirit {

namespace arduino {

DigitalOut::DigitalOut(const uint8_t pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

DigitalOut::~DigitalOut()
{
}

void DigitalOut::write(const uint32_t value)
{
    switch (value) {
        case 0:
            digitalWrite(_pin, LOW);
            break;
        case 1:
        default:
            digitalWrite(_pin, HIGH);
            break;
    }
}

uint32_t DigitalOut::read()
{
    return digitalRead(_pin);
}

}  // namespace arduino

}  // namespace spirit

#endif  // ARDUINO
