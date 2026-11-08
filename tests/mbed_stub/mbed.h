#ifndef SPIRIT_TEST_MBED_STUB_H
#define SPIRIT_TEST_MBED_STUB_H

// mbed.h のスタブ (ホスト上で Mbed 用のアダプターをオフラインでテストするため)
// Mbed OS 6 / Mbed CE の API のうち、アダプターが使うものだけを、同じシグネチャで再現している

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <vector>

namespace mbed {

enum PinName { NC = -1, PA_0 = 0, PA_1 = 1, PA_2 = 2, PA_3 = 3 };

/// コールバック (本物と同様に、空の状態を持ち、ヒープを使わないが、スタブでは std::function で代用する)
template <typename T>
class Callback;

template <>
class Callback<void()> {
public:
    Callback() = default;

    Callback(std::nullptr_t)
    {
    }

    Callback(std::function<void()> function) : _function(function)
    {
    }

    explicit operator bool() const
    {
        return static_cast<bool>(_function);
    }

    void operator()() const
    {
        if (_function) {
            _function();
        }
    }

private:
    std::function<void()> _function;
};

template <typename T>
Callback<void()> callback(T* object, void (T::*method)())
{
    return Callback<void()>([object, method]() { (object->*method)(); });
}

/// ピンの状態の記録
struct PinState {
    int   digital{0};
    float duty_cycle{0.0F};
    float period{0.0F};
};

inline std::map<PinName, PinState>& pin_states()
{
    static std::map<PinName, PinState> states;
    return states;
}

class DigitalOut {
public:
    explicit DigitalOut(PinName pin) : _pin(pin)
    {
    }

    void write(int value)
    {
        pin_states()[_pin].digital = value;
    }

    int read()
    {
        return pin_states()[_pin].digital;
    }

private:
    PinName _pin;
};

class PwmOut {
public:
    explicit PwmOut(PinName pin) : _pin(pin)
    {
    }

    void write(float value)
    {
        pin_states()[_pin].duty_cycle = value;
    }

    float read()
    {
        return pin_states()[_pin].duty_cycle;
    }

    void period(float seconds)
    {
        pin_states()[_pin].period = seconds;
    }

private:
    PinName _pin;
};

class InterruptIn;

inline std::map<PinName, InterruptIn*>& interrupt_pins()
{
    static std::map<PinName, InterruptIn*> pins;
    return pins;
}

class InterruptIn {
public:
    explicit InterruptIn(PinName pin) : _pin(pin)
    {
        interrupt_pins()[pin] = this;
    }

    ~InterruptIn()
    {
        interrupt_pins().erase(_pin);
    }

    void rise(Callback<void()> function)
    {
        _rise = function;
    }

    void fall(Callback<void()> function)
    {
        _fall = function;
    }

    int read()
    {
        return pin_states()[_pin].digital;
    }

    /// 割り込みを発生させる(ピンの状態を変えて、登録された関数を呼び出す)
    void fire(bool rising)
    {
        pin_states()[_pin].digital = rising ? 1 : 0;
        (rising ? _rise : _fall)();
    }

    bool has_rise() const
    {
        return static_cast<bool>(_rise);
    }

    bool has_fall() const
    {
        return static_cast<bool>(_fall);
    }

private:
    PinName          _pin;
    Callback<void()> _rise;
    Callback<void()> _fall;
};

}  // namespace mbed

using namespace mbed;

/// wait_us() の呼び出しの記録
inline std::vector<int>& wait_us_calls()
{
    static std::vector<int> calls;
    return calls;
}

inline void wait_us(int microseconds)
{
    wait_us_calls().push_back(microseconds);
}

#endif  // SPIRIT_TEST_MBED_STUB_H
