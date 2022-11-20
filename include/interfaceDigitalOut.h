#ifndef SPIRIT_INTERFACE_DIGITALOUT_H
#define SPIRIT_INTERFACE_DIGITALOUT_H

#include <cstdint>

namespace spirit {

class interfaceDigitalOut {
public:
    virtual void write(uint32_t value) = 0;

    virtual uint32_t read() const = 0;
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_DIGITALOUT_H
