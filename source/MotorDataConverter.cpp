#include "MotorDataConverter.h"

#include "Motor.h"
#include "PwmDataConverter.h"
#include "SpeedDataConverter.h"
#include "include/Error.h"

namespace spirit {

bool MotorDataConverter::encode(const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
                                std::size_t& buffer_size)
{
    switch (motor.get_control_system()) {
        case Motor::ControlSystem::PWM:
            PwmDataConverter pwm_data_converter;
            return pwm_data_converter.encode(motor, max_buffer_size, buffer, buffer_size);
            break;
        case Motor::ControlSystem::Speed:
            SpeedDataConverter speed_data_converter;
            return speed_data_converter.encode(motor, max_buffer_size, buffer, buffer_size);
        // default になることは、 motor 側で事前にチェックしているのでありえないため、カバレッジ計測から除外する
        // LCOV_EXCL_START
        default:
            constexpr char message_format[] = "Unknown motor control system (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(motor.get_control_system()));
            Error::get_instance().error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            return false;
            // LCOV_EXCL_STOP
    }
}

bool MotorDataConverter::decode(const uint8_t* buffer, const std::size_t buffer_size, Motor& motor)
{
    switch (buffer[0] & 0xC0) {
        case 0x00:
            PwmDataConverter pwm_data_converter;
            return pwm_data_converter.decode(buffer, buffer_size, motor);
        case 0x40:
            SpeedDataConverter speed_data_converter;
            return speed_data_converter.decode(buffer, buffer_size, motor);
        default:
            return false;
    }
}

}  // namespace spirit
