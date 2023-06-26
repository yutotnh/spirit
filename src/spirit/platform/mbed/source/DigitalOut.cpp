#ifdef __MBED__

#include "spirit/platform/mbed/include/DigitalOut.h"

#include "mbed.h"

namespace spirit {

namespace mbed {

DigitalOut::DigitalOut(PinName pin) : _digital_out(pin)
{
}

DigitalOut::~DigitalOut()
{
}

void DigitalOut::write(const uint32_t value)
{
    _digital_out.write(value);
}

uint32_t DigitalOut::read()
{
    return _digital_out.read();
}

}  // namespace mbed

}  // namespace spirit

#endif  // __MBED__
