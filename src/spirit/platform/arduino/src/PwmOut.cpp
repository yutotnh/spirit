#ifdef ARDUINO

#include "spirit/platform/arduino/include/PwmOut.h"

namespace spirit {

namespace arduino {

PwmOut::PwmOut(const uint8_t pin, const uint8_t channel)
{
    _pin = pin;

    // _channelは未使用
    // ledcを利用する時に必要なので、あらかじめ宣言しておく
    _channel = channel;
}

PwmOut::~PwmOut()
{
}

void PwmOut::write(const float value)
{
    // value の範囲は0.0 - 1.0で、dacWriteとanalogWriteの範囲は0 - 255なので変換する
#ifdef ESP32
    dacWrite(_pin, value * 255);
#else
    analogWrite(_pin, value * 255);
#endif

    if (value < 0.00F) {
        _duty_cycle = 0.00F;
    } else if (1.00F < value) {
        _duty_cycle = 1.00F;
    } else {
        _duty_cycle = value;
    }
}

float PwmOut::read()
{
    return _duty_cycle;
}

void PwmOut::period(const float seconds)
{
    // TODO 将来ledcを利用して実装する
}

}  // namespace arduino

}  // namespace spirit

#endif  // ARDUINO
