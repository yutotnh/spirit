#ifdef ARDUINO

#ifndef SPIRIT_ARDUINO_DELAY_H
#define SPIRIT_ARDUINO_DELAY_H

#include "Arduino.h"
#include "spirit/include/InterfaceDelay.h"

namespace spirit {

namespace arduino {

/**
 * @brief Arduino の delayMicroseconds() を使った InterfaceDelay
 */
class Delay : public InterfaceDelay {
public:
    void delay_us(uint32_t microseconds) override;
};

}  // namespace arduino

}  // namespace spirit

#endif  // SPIRIT_ARDUINO_DELAY_H

#endif  // ARDUINO
