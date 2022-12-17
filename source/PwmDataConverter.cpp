#include "PwmDataConverter.h"

#include <cstring>

#include "bit.h"
namespace spirit {

bool PwmDataConverter::encode(const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
                              std::size_t& buffer_size)
{
    // PWM制御を示すヘッダをセット(既に0で初期化されているので不要)
    // buffer[0] = 0x00;

    buffer_size = 3;

    if (max_buffer_size < buffer_size) {
        return false;
    }

    set_duty_cycle(motor.get_duty_cycle(), buffer);

    set_state(motor.get_state(), buffer);

    return true;
}

bool PwmDataConverter::decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor)
{
    const std::size_t required_size = 3;
    if (buffer_size < required_size) {
        return false;
    }

    motor.duty_cycle(get_duty_cycle(buffer));
    motor.state(get_state(buffer));

    return true;
}

float PwmDataConverter::get_duty_cycle(const uint8_t* buffer)
{
    const uint16_t duty_cycle_16bit = get_range_value(buffer, 8, 2, 16);
    return duty_cycle_16bit / 65535.0F;
}

void PwmDataConverter::set_duty_cycle(const float duty_cycle, uint8_t* buffer)
{
    const uint16_t duty_cycle_16bit = 65535 * duty_cycle;  // 2^16 - 1 = 65535
    set_range_value(duty_cycle_16bit, 2, 16, 8, buffer);
}

Motor::State PwmDataConverter::get_state(const uint8_t* buffer)
{
    uint8_t state_uint8_t = get_range_value(buffer, 8, 18, 2);

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

void PwmDataConverter::set_state(const Motor::State state, uint8_t* buffer)
{
    switch (state) {
        case Motor::State::Coast:
            // buffer[2] |= 0x00 << 5; // 既に0で初期化されているので不要
            break;
        case Motor::State::CW:
            set_range_value(0x01U, 18U, 2U, 8U, buffer);
            break;
        case Motor::State::CCW:
            set_range_value(0x02U, 18U, 2U, 8U, buffer);
            break;
        case Motor::State::Brake:
            set_range_value(0x03U, 18U, 2U, 8U, buffer);
            break;
        default:
            break;
    }
}

}  // namespace spirit
