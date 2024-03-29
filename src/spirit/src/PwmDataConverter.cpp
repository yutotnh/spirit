#include "spirit/include/PwmDataConverter.h"

#include <cstdio>

#include "spirit/include/Error.h"
#include "spirit/include/bit.h"

namespace spirit {

bool PwmDataConverter::encode(const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
                              std::size_t& buffer_size) const
{
    // 2(ヘッダ) + 16(デューティー比) + 2(回転方向) = 20
    buffer_size = 20;

    if (max_buffer_size < buffer_size) {
        return false;
    }

    // PWM制御を示すヘッダをセット
    buffer[0] = 0x00;

    set_duty_cycle(motor.get_duty_cycle(), buffer);

    set_state(motor.get_state(), buffer);

    return true;
}

bool PwmDataConverter::decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor) const
{
    // 2(ヘッダ) + 16(デューティー比) + 2(回転方向) = 20
    constexpr std::size_t required_size = 20;
    if (buffer_size < required_size) {
        return false;
    }

    if ((buffer[0] & 0xC0) != 0x00) {
        return false;
    }

    motor.control_system(Motor::ControlSystem::PWM);
    motor.duty_cycle(get_duty_cycle(buffer));
    motor.state(get_state(buffer));

    return true;
}

float PwmDataConverter::get_duty_cycle(const uint8_t* buffer) const
{
    uint32_t duty_cycle_16bit = 0;
    get_range_value(buffer, 8, 2, 16, duty_cycle_16bit);
    return duty_cycle_16bit / 65535.0F;
}

void PwmDataConverter::set_duty_cycle(const float duty_cycle, uint8_t* buffer) const
{
    /// @todo デューティー比がマイナスを取ることはないため、その場合はエラーにする

    const uint16_t duty_cycle_16bit = (uint16_t)(65535 * duty_cycle);  // 2^16 - 1 = 65535
    set_range_value(duty_cycle_16bit, 2, 16, 8, buffer);
}

Motor::State PwmDataConverter::get_state(const uint8_t* buffer) const
{
    uint32_t   state_uint32_t = 0;
    const bool is_normal      = get_range_value(buffer, 8, 18, 2, state_uint32_t);

    if (!is_normal) {
        return Motor::State::Brake;
    }

    switch (state_uint32_t) {
        case 0x00U:
            return Motor::State::Coast;
        case 0x01U:
            return Motor::State::CW;
        case 0x02U:
            return Motor::State::CCW;
        case 0x03U:
            return Motor::State::Brake;

            // default に来ることは、取得しているビット幅が2bitでありcase文0-3までで処理が決まっていてありえないため、カバレッジ計測から除外する
            // LCOV_EXCL_START
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor state (%d)", state_uint32_t);
            return Motor::State::Brake;
            // LCOV_EXCL_STOP
    }
}

void PwmDataConverter::set_state(const Motor::State state, uint8_t* buffer) const
{
    switch (state) {
        case Motor::State::Coast:
            // buffer[2] |= 0x00 << 5; // 既に0で初期化されているので不要
            return;
        case Motor::State::CW:
            set_range_value(0x01U, 18U, 2U, 8U, buffer);
            return;
        case Motor::State::CCW:
            set_range_value(0x02U, 18U, 2U, 8U, buffer);
            return;
        case Motor::State::Brake:
            set_range_value(0x03U, 18U, 2U, 8U, buffer);
            return;

            // default に来ることは、state で既にチェックしているので通常の利用ではありえないため、カバレッジ計測から除外する
            // LCOV_EXCL_START
        default:
            Error::get_instance().error(Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__,
                                        "Unknown motor state (%d)", static_cast<uint32_t>(state));
            return;
            // LCOV_EXCL_STOP
    }
}

}  // namespace spirit
