#ifndef SPIRIT_MOTOR_PROTOCOL_CODER_H
#define SPIRIT_MOTOR_PROTOCOL_CODER_H

#include <stdint.h>

#include "Motor.h"

namespace spirit {

namespace MotorProtocolCoder {

enum class Type {
    Pwm,
};

bool encode(const Type type, const Motor& motor, const std::size_t max_buffer_size, uint8_t* buffer,
            std::size_t& buffer_size);

bool decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor);

}  // namespace MotorProtocolCoder

}  // namespace spirit

#endif  // SPIRIT_MOTOR_PROTOCOL_CODER_H
