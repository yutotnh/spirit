#ifdef __MBED__

#include "spirit/platform/mbed/include/Delay.h"

#include <climits>

#include "mbed.h"

namespace spirit {

namespace mbed {

void Delay::delay_us(const uint32_t microseconds)
{
    // wait_us() の引数は int なので、範囲を超える場合は、収める
    wait_us((microseconds > static_cast<uint32_t>(INT_MAX)) ? INT_MAX : static_cast<int>(microseconds));
}

}  // namespace mbed

}  // namespace spirit

#endif  // __MBED__
