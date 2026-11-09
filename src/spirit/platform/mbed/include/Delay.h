#ifndef SPIRIT_MBED_DELAY_H
#define SPIRIT_MBED_DELAY_H

#include "mbed.h"
#include "spirit/include/InterfaceDelay.h"

namespace spirit {

namespace mbed {

/**
 * @brief Mbed の wait_us() を使った InterfaceDelay
 */
class Delay : public InterfaceDelay {
public:
    void delay_us(uint32_t microseconds) override;
};

}  // namespace mbed

}  // namespace spirit

#endif  // SPIRIT_MBED_DELAY_H
