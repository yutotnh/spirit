#include "SpeedDataConverter.h"

#include "bfloat16.h"
#include "bit.h"
#include "include/Error.h"

namespace spirit {

bool SpeedDataConverter::encode(const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
                                std::size_t& buffer_size)
{
    // 2(ヘッダ) + 16(スピード(rps)) + 16(Kp) + 16(Ki) + 2(回転方向) = 52
    buffer_size = 52;

    if (max_buffer_size < buffer_size) {
        return false;
    }

    // 速度制御を示すヘッダをセット
    buffer[0] = 0x40;

    set_speed(motor.get_speed(), buffer);

    float Kp{0.0F}, Ki{0.0F}, Kd{0.0F};
    motor.get_pid_gain_factor(Kp, Ki, Kd);

    set_pid_gain_factor(Kp, Ki, Kd, buffer);

    set_state(motor.get_state(), buffer);

    return true;
}

bool SpeedDataConverter::decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor)
{
    if (buffer_size < 7) {
        return false;
    }

    if ((buffer[0] & 0xC0) != 0x40) {
        return false;
    }

    motor.control_system(Motor::ControlSystem::Speed);

    motor.speed(get_speed(buffer));

    float Kp{0.0F}, Ki{0.0F}, Kd{0.0F};
    get_pid_gain_factor(buffer, Kp, Ki, Kd);
    motor.pid_gain_factor(Kp, Ki, Kd);

    motor.state(get_state(buffer));

    return true;
}

float SpeedDataConverter::get_speed(const uint8_t* buffer)
{
    const uint16_t speed_16bit = get_range_value(buffer, 8, 2, 16);
    return bfloat16_to_float32(speed_16bit);
}

void SpeedDataConverter::set_speed(float speed, uint8_t* buffer)
{
    uint16_t speed_16bit = float32_to_bfloat16(speed);
    set_range_value(speed_16bit, 2, 16, 7, buffer);
}

void SpeedDataConverter::get_pid_gain_factor(const uint8_t* buffer, float& Kp, float& Ki, float& Kd)
{
    // Kdは送受信しないので処理しない
    (void)Kd;

    const uint16_t Kp_16bit = get_range_value(buffer, 7, 18, 16);
    Kp                      = bfloat16_to_float32(Kp_16bit);

    const uint16_t Ki_16bit = get_range_value(buffer, 7, 34, 16);
    Ki                      = bfloat16_to_float32(Ki_16bit);
}

void SpeedDataConverter::set_pid_gain_factor(float Kp, float Ki, float Kd, uint8_t* buffer)
{
    // Kdは送受信しないので処理しない
    (void)Kd;

    uint16_t Kp_16bit = float32_to_bfloat16(Kp);
    set_range_value(Kp_16bit, 18, 16, 7, buffer);

    uint16_t Ki_16bit = float32_to_bfloat16(Ki);
    set_range_value(Ki_16bit, 34, 16, 7, buffer);
}

Motor::State SpeedDataConverter::get_state(const uint8_t* buffer)
{
    constexpr uint32_t start  = 50;
    constexpr uint32_t length = 2;
    const uint8_t      state  = get_range_value(buffer, 7, start, length);
    switch (state) {
        case 0x00:
            return Motor::State::Coast;
        case 0x01:
            return Motor::State::CW;
        case 0x02:
            return Motor::State::CCW;
        case 0x03:
            return Motor::State::Brake;

            // default に来ることは、取得しているビット幅が2bitでありcase文0-3までで処理が決まっていてありえないため、カバレッジ計測から除外する
            // LCOV_EXCL_START
        default:
            constexpr char message_format[] = "Unknown motor state (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(state));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            return Motor::State::Brake;
            // LCOV_EXCL_STOP
    }
}

void SpeedDataConverter::set_state(Motor::State state, uint8_t* buffer)
{
    constexpr uint32_t start       = 50;
    constexpr uint32_t length      = 2;
    constexpr uint32_t buffer_size = 7;
    switch (state) {
        case Motor::State::Coast:
            // buffer[7] |= 0x00 << 4;  // 既に0で初期化されているので不要
            break;
        case Motor::State::CW:
            set_range_value(0x01U, start, length, buffer_size, buffer);
            break;
        case Motor::State::CCW:
            set_range_value(0x02U, start, length, buffer_size, buffer);
            break;
        case Motor::State::Brake:
            set_range_value(0x03U, start, length, buffer_size, buffer);
            break;

            // default に来ることは、state で既にチェックしているので通常の利用ではありえないため、カバレッジ計測から除外する
            // LCOV_EXCL_START
        default:
            constexpr char message_format[] = "Unknown motor state (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(state));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            break;
            // LCOV_EXCL_STOP
    }
}

}  // namespace spirit
