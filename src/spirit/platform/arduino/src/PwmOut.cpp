#ifdef ARDUINO

#include "spirit/platform/arduino/include/PwmOut.h"

#include <cmath>

namespace spirit {

namespace arduino {

namespace {

#ifdef ESP32
/// LEDC のデューティー比の分解能(bit) @n
/// 80MHz / 2^10 = 78kHz なので、 Motor::min_pulse_period (60kHz) までの周期に対応できる
constexpr uint8_t ledc_resolution = 10;

/// LEDC のデューティー比の最大値
constexpr uint32_t ledc_max_duty = (1U << ledc_resolution) - 1U;

/// Arduino core for ESP32 のバージョン3.0以降は、チャンネルの指定が不要な API になっている
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
#define SPIRIT_ESP32_LEDC_PIN_API
#endif

#endif  // ESP32

}  // namespace

PwmOut::PwmOut(const uint8_t pin, const uint8_t channel)
{
    _pin     = pin;
    _channel = channel;

#ifdef ESP32
    const float frequency = 1.0F / _period;
#ifdef SPIRIT_ESP32_LEDC_PIN_API
    (void)_channel;
    ledcAttach(_pin, static_cast<uint32_t>(frequency), ledc_resolution);
#else
    ledcSetup(_channel, frequency, ledc_resolution);
    ledcAttachPin(_pin, _channel);
#endif
    output();
#endif
}

PwmOut::~PwmOut()
{
}

void PwmOut::write(const float value)
{
    // NaN は 0 として扱う
    if (!(0.00F < value)) {
        _duty_cycle = 0.00F;
    } else if (1.00F < value) {
        _duty_cycle = 1.00F;
    } else {
        _duty_cycle = value;
    }

    output();
}

float PwmOut::read()
{
    return _duty_cycle;
}

void PwmOut::period(const float seconds)
{
    // NaN や 0 以下は無視する
    if (!(0.00F < seconds)) {
        return;
    }

    _period = seconds;

#ifdef ESP32
    const float frequency = 1.0F / _period;
#ifdef SPIRIT_ESP32_LEDC_PIN_API
    ledcChangeFrequency(_pin, static_cast<uint32_t>(frequency), ledc_resolution);
#else
    ledcSetup(_channel, frequency, ledc_resolution);
#endif
    // 周期を変えるとデューティー比の設定が失われることがあるので、設定し直す
    output();
#endif
}

float PwmOut::read_period() const
{
    return _period;
}

void PwmOut::output() const
{
#ifdef ESP32
    // dacWrite() は PWM ではなく DAC (対応ピンは GPIO25/26 のみ)なので使わない
    const uint32_t duty = static_cast<uint32_t>(_duty_cycle * static_cast<float>(ledc_max_duty) + 0.5F);
#ifdef SPIRIT_ESP32_LEDC_PIN_API
    ledcWrite(_pin, duty);
#else
    ledcWrite(_channel, duty);
#endif
#else
    // analogWrite() の範囲は 0 - 255 なので変換する
    analogWrite(_pin, static_cast<int>(_duty_cycle * 255.0F + 0.5F));
#endif
}

}  // namespace arduino

}  // namespace spirit

#endif  // ARDUINO
