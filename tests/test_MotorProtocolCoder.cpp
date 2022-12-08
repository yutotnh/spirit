#include <gtest/gtest.h>

#include "Motor.h"
#include "MotorProtocolCoder.h"

using namespace spirit;

TEST(MotorProtocolCoder, EncodeDecodePwmTest)
{
    auto encode_decode = [](float duty_cycle, Motor::State state) {
        Motor motor;
        motor.duty_cycle(duty_cycle);
        motor.state(state);

        constexpr std::size_t max_buffer_size = 8;
        uint8_t               buffer[max_buffer_size]{};
        std::size_t           buffer_size = 0;

        MotorProtocolCoder::encode(MotorProtocolCoder::Type::Pwm, motor, max_buffer_size, buffer, buffer_size);

        Motor decoded_motor;
        MotorProtocolCoder::decode(buffer, max_buffer_size, decoded_motor);

        // デューティ比は0.0~1.0の範囲を(2^16 - 1)で割った値を送受信しているでため、誤差は1/(2^16 - 1)とする
        constexpr float allowable_error_margin = 1.0F / 65535.0F;
        EXPECT_NEAR(duty_cycle, decoded_motor.get_duty_cycle(), allowable_error_margin);
        EXPECT_EQ(state, decoded_motor.get_state());
    };

    // デューティ比が0未満や1より大きい場合のテストはMotorクラスのテストで行う
    // とりあえず、デューティ比が最小/中間/適当な細かい値/最大と、各回転方向のテストを行う
    encode_decode(0.00F, Motor::State::Coast);
    encode_decode(0.00F, Motor::State::CW);
    encode_decode(0.00F, Motor::State::CCW);
    encode_decode(0.00F, Motor::State::Brake);

    encode_decode(0.50F, Motor::State::Coast);
    encode_decode(0.50F, Motor::State::CW);
    encode_decode(0.50F, Motor::State::CCW);
    encode_decode(0.50F, Motor::State::Brake);

    encode_decode(0.123456F, Motor::State::Coast);
    encode_decode(0.123456F, Motor::State::CW);
    encode_decode(0.123456F, Motor::State::CCW);
    encode_decode(0.123456F, Motor::State::Brake);

    encode_decode(1.00F, Motor::State::Coast);
    encode_decode(1.00F, Motor::State::CW);
    encode_decode(1.00F, Motor::State::CCW);
    encode_decode(1.00F, Motor::State::Brake);
}
