#ifdef ARDUINO

#include "spirit/platform/arduino/include/Delay.h"

namespace spirit {

namespace arduino {

void Delay::delay_us(const uint32_t microseconds)
{
    delayMicroseconds(microseconds);
}

}  // namespace arduino

}  // namespace spirit

#endif  // ARDUINO
