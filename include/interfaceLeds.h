#ifndef SPIRIT_INTERFACE_LEDS_H
#define SPIRIT_INTERFACE_LEDS_H

#include <cstdint>

namespace spirit {

class interfaceLeds {
public:
    virtual void write(const uint32_t value) = 0;

    virtual uint32_t read() const = 0;
};

}  // namespace spirit

#endif  // SPIRIT_INTERFACE_LEDS_H
