#ifndef SPIRIT_TEST_ARDUINO_STUB_H
#define SPIRIT_TEST_ARDUINO_STUB_H

// Arduino.h のスタブ (ホスト上で Arduino 用のコードをオフラインでテストするため)
// 関数の呼び出しを記録し、 digitalRead() の値は設定できる

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#define LOW                        0x0
#define HIGH                       0x1
#define OUTPUT                     0x1
#define RISING                     0x01
#define FALLING                    0x02
#define CHANGE                     0x03
#define digitalPinToInterrupt(pin) (pin)

namespace arduino_stub {

/// 呼び出しの記録
inline std::vector<std::string>& calls()
{
    static std::vector<std::string> value;
    return value;
}

/// digitalRead() が返す値 (ピンごと。設定していないピンは LOW)
inline std::map<int, int>& digital_read_values()
{
    static std::map<int, int> values;
    return values;
}

/// attachInterrupt() で登録された割り込み
struct Interrupt {
    void (*function)();
    int mode;
};

inline std::map<int, Interrupt>& interrupts()
{
    static std::map<int, Interrupt> values;
    return values;
}

/// ピンの割り込みを発生させる(登録された関数を呼び出す)。登録されていなければ false
inline bool fire_interrupt(int pin)
{
    auto it = interrupts().find(pin);
    if (it == interrupts().end()) {
        return false;
    }
    it->second.function();
    return true;
}

inline void record(const std::string& name, long a = 0, long b = 0, long c = 0)
{
    calls().push_back(name + "(" + std::to_string(a) + "," + std::to_string(b) + "," + std::to_string(c) + ")");
}

}  // namespace arduino_stub

inline void pinMode(uint8_t pin, uint8_t mode)
{
    arduino_stub::record("pinMode", pin, mode);
}

inline void digitalWrite(uint8_t pin, uint8_t value)
{
    arduino_stub::record("digitalWrite", pin, value);
}

inline int digitalRead(uint8_t pin)
{
    return arduino_stub::digital_read_values()[pin];
}

inline void attachInterrupt(uint8_t interrupt, void (*function)(), int mode)
{
    arduino_stub::record("attachInterrupt", interrupt, mode);
    arduino_stub::interrupts()[interrupt] = {function, mode};
}

inline void detachInterrupt(uint8_t interrupt)
{
    arduino_stub::record("detachInterrupt", interrupt);
    arduino_stub::interrupts().erase(interrupt);
}

inline void delayMicroseconds(unsigned int microseconds)
{
    arduino_stub::record("delayMicroseconds", microseconds);
}

inline void analogWrite(uint8_t pin, int value)
{
    arduino_stub::record("analogWrite", pin, value);
}

inline void dacWrite(uint8_t pin, uint8_t value)
{
    arduino_stub::record("dacWrite", pin, value);
}

// Arduino core for ESP32 2.x の LEDC API
inline double ledcSetup(uint8_t channel, double freq, uint8_t resolution)
{
    arduino_stub::record("ledcSetup", channel, static_cast<long>(freq), resolution);
    return freq;
}

inline void ledcAttachPin(uint8_t pin, uint8_t channel)
{
    arduino_stub::record("ledcAttachPin", pin, channel);
}

// Arduino core for ESP32 2.x では channel、3.x では pin を指定する(どちらも第1引数)
inline void ledcWrite(uint8_t channel_or_pin, uint32_t duty)
{
    arduino_stub::record("ledcWrite", channel_or_pin, duty);
}

// Arduino core for ESP32 3.x の LEDC API
inline bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution)
{
    arduino_stub::record("ledcAttach", pin, freq, resolution);
    return true;
}

inline uint32_t ledcChangeFrequency(uint8_t pin, uint32_t freq, uint8_t resolution)
{
    arduino_stub::record("ledcChangeFrequency", pin, freq, resolution);
    return freq;
}

#endif  // SPIRIT_TEST_ARDUINO_STUB_H
