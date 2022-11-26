#ifndef SPIRIT_STUB_H
#define SPIRIT_STUB_H

#include "InterfaceDigitalOut.h"
#include "InterfacePwmOut.h"

namespace spirit {

class StubDigitalOut : public InterfaceDigitalOut {
public:
    void write(const uint32_t value) override
    {
        _value = value;
    }

    uint32_t read() const override
    {
        return _value;
    }

private:
    uint32_t _value{0};
};

class StubPwmOut : public InterfacePwmOut {
public:
    void write(const float value) override
    {
        _value = value;
    }

    float read() const override
    {
        return _value;
    }

    void period(const float seconds) override
    {
        _period = seconds;
    }

    float read_period()
    {
        return _period;
    }

private:
    float _value{0.00F};
    float _period{0.00F};
};

}  // namespace spirit

#endif  // SPIRIT_STUB_H
