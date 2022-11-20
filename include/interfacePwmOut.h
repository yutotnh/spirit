#ifndef SPIRIT_INTERFACE_PWMOUT_H
#define SPIRIT_INTERFACE_PWMOUT_H

#include <cstdint>

namespace spirit {

class interfacePwmOut {
public:
    virtual void write(float value) = 0;

    virtual float read() const = 0;

    virtual void period(float seconds) = 0;
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_PWMOUT_H
