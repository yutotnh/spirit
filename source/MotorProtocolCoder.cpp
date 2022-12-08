#include "MotorProtocolCoder.h"

#include <cstring>
namespace spirit {

namespace MotorProtocolCoder {

namespace {
bool         encode_pwm(const Motor& motor, uint8_t* buffer, std::size_t buffer_size);
bool         decode_pwm(const uint8_t* buffer, std::size_t buffer_size, Motor& motor);
float        get_duty_cycle(const uint8_t* buffer);
void         set_duty_cycle(float duty_cycle, uint8_t* buffer);
Motor::State get_state(const uint8_t* buffer);
void         set_state(const Motor::State state, uint8_t* buffer);
uint32_t     get_specified_range_value(const uint8_t* buffer, const std::size_t buffer_size, const std::size_t start,
                                       const std::size_t value_size);
bool         set_specified_range_value(const uint32_t value, const std::size_t start, const std::size_t value_size,
                                       const std::size_t buffer_size, uint8_t* buffer);
}  // namespace

// TODO buffer_size は 8 で決めうちにする? 返り値でサイズを返す?
// 8byteなのはCANだけ、CAN, I2Cとかに対応させることを考えるとMAXも与えないといけない
bool encode(const Type type, const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
            std::size_t& buffer_size)
{
    buffer_size = 5;
    memset(buffer, 0, max_buffer_size);
    switch (type) {
        case Type::Pwm:
            encode_pwm(motor, buffer, max_buffer_size);
            break;

        default:
            break;
    }
    return true;
}

bool decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor)
{
    const auto type = (buffer[0] & 0xC0U) >> 6;
    switch (0) {
        case 0x00U:
            decode_pwm(buffer, buffer_size, motor);
            break;

        default:
            break;
    }
    return true;
}

namespace {

bool encode_pwm(const Motor& motor, uint8_t* buffer, std::size_t buffer_size)
{
    if (buffer_size < 1) {
        return false;
    }

    // PWM制御を示すのヘッダをセット
    // buffer[0] = 0x00; // 既に0で初期化されているので不要

    set_duty_cycle(motor.get_duty_cycle(), buffer);

    set_state(motor.get_state(), buffer);

    return true;
}

bool decode_pwm(const uint8_t* buffer, std::size_t buffer_size, Motor& motor)
{
    if (buffer_size < 1) {
        return false;
    }

    motor.duty_cycle(get_duty_cycle(buffer));
    motor.state(get_state(buffer));

    return true;
}

float get_duty_cycle(const uint8_t* buffer)
{
    const uint16_t duty_cycle_16bit = get_specified_range_value(buffer, 8, 2, 16);
    return duty_cycle_16bit / 65535.0F;
}

void set_duty_cycle(const float duty_cycle, uint8_t* buffer)
{
    const uint16_t duty_cycle_16bit = 65535 * duty_cycle;  // 2^16 - 1 = 65535
    set_specified_range_value(duty_cycle_16bit, 2, 16, 8, buffer);
}

Motor::State get_state(const uint8_t* buffer)
{
    uint8_t state_uint8_t = get_specified_range_value(buffer, 8, 18, 2);

    switch (state_uint8_t) {
        case 0x00U:
            return Motor::State::Coast;
        case 0x01U:
            return Motor::State::CW;
        case 0x02U:
            return Motor::State::CCW;
        case 0x03U:
            return Motor::State::Brake;
        default:
            return Motor::State::Brake;
            break;
    }
}

void set_state(const Motor::State state, uint8_t* buffer)
{
    switch (state) {
        case Motor::State::Coast:
            // buffer[2] |= 0x00 << 5; // 既に0で初期化されているので不要
            break;
        case Motor::State::CW:
            set_specified_range_value(0x01U, 18U, 2U, 8U, buffer);
            break;
        case Motor::State::CCW:
            set_specified_range_value(0x02U, 18U, 2U, 8U, buffer);
            break;
        case Motor::State::Brake:
            set_specified_range_value(0x03U, 18U, 2U, 8U, buffer);
            break;
        default:
            break;
    }
}

uint32_t get_specified_range_value(const uint8_t* buffer, const std::size_t buffer_size, const std::size_t start,
                                   const std::size_t value_size)
{
    auto end = start + value_size - 1;
    if ((buffer_size * __CHAR_BIT__) < end) {
        return UINT32_MAX;
    }

    uint32_t value = 0;

    for (std::size_t i = start; i <= end; ++i) {
        const auto byte  = i / 8;  // 最上位バイトから数える(0〜)
        const auto bit   = i % 8;  // 最上位ビットから数える(0〜7)
        const auto shift = 7 - bit;

        value <<= 1;
        value |= ((buffer[byte] >> shift) & 0x01);
    }
    return value;
}

bool set_specified_range_value(const uint32_t value, const std::size_t start, const std::size_t value_size,
                               const std::size_t buffer_size, uint8_t* buffer)
{
    auto end = start + value_size - 1;
    if ((buffer_size * __CHAR_BIT__) < end) {
        return false;
    }

    for (std::size_t i = start; i <= end; ++i) {
        const auto byte         = i / 8;  // 最上位バイトから数える(0〜)
        const auto bit          = i % 8;  // 最上位ビットから数える(0〜7)
        const auto buffer_shift = 7 - bit;
        const auto value_shift  = end - i;

        const auto _1bit = (value >> value_shift) & 0x01;

        if (_1bit == 1) {
            buffer[byte] |= (1 << buffer_shift);
        } else {
            buffer[byte] &= ~(1 << buffer_shift);
        }
    }

    return true;
}

}  // namespace

}  // namespace MotorProtocolCoder

}  // namespace spirit
