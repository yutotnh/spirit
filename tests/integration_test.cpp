#include <gtest/gtest.h>

#include "FakeUdpConverter.h"
#include "Id.h"
#include "Motor.h"
#include "PwmDataConverter.h"
#include "include/Error.h"

namespace {

struct CANMessage {
    uint32_t    id;
    uint8_t     data[8];
    std::size_t len;
};

/**
 * @brief 簡易的な結合テスト(デューティー比とモータの状態を送信し、受信側でモータの状態とデューティー比を比較する)
 */
TEST(IntegrationTest, 1)
{
    constexpr uint32_t dip_sw = 0x00;

    /// 送信側
    auto controller = [&dip_sw](spirit::Motor &motor) {
        spirit::FakeUdpConverter fake_udp;
        spirit::PwmDataConverter pwm_data;
        const uint32_t           can_id = spirit::can::get_motor_id(1, 0, dip_sw);

        constexpr std::size_t max_pwm_data_buffer_size = 64;
        uint8_t               pwm_data_buffer[8]       = {};
        std::size_t           pwm_data_buffer_size;

        constexpr std::size_t max_fake_udp_buffer_size = 64;
        uint8_t               fake_udp_buffer[8]       = {};
        std::size_t           fake_udp_buffer_size;

        pwm_data.encode(motor, max_pwm_data_buffer_size, pwm_data_buffer, pwm_data_buffer_size);

        fake_udp.encode(pwm_data_buffer, pwm_data_buffer_size, max_fake_udp_buffer_size, fake_udp_buffer,
                        fake_udp_buffer_size);

        ::CANMessage message;
        message.id = can_id;
        if (fake_udp_buffer_size % 8) {
            message.len = (fake_udp_buffer_size / 8) + 1;
        } else {
            message.len = fake_udp_buffer_size / 8;
        }

        memmove(message.data, fake_udp_buffer, 8);

        return message;
    };

    /// 受信側
    auto peripheral = [&dip_sw](const ::CANMessage &message) {
        spirit::FakeUdpConverter fake_udp;
        spirit::PwmDataConverter pwm_data;

        const uint32_t can_id = spirit::can::get_motor_id(1, 0, dip_sw);

        EXPECT_EQ(message.id, can_id);

        constexpr std::size_t max_payload_size = 64;
        uint8_t               payload[8]       = {};
        std::size_t           payload_size;
        fake_udp.decode(message.data, message.len * 8, max_payload_size, payload, payload_size);

        spirit::Motor motor;
        pwm_data.decode(payload, payload_size, motor);

        return motor;
    };

    /// テスト
    auto test = [&controller, &peripheral](spirit::Motor::State state, float duty_cycle) {
        spirit::Motor motor;
        motor.state(state);
        motor.duty_cycle(duty_cycle);

        ::CANMessage  message        = controller(motor);
        spirit::Motor received_motor = peripheral(message);

        // デューティー比は16bitで表現しているため、1/65535の誤差を許容する
        constexpr float allowable_error_margin = 1.0F / 65535.0F;
        EXPECT_EQ(state, received_motor.get_state());
        EXPECT_NEAR(duty_cycle, received_motor.get_duty_cycle(), allowable_error_margin);
    };

    spirit::Error &error = spirit::Error::get_instance();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    /// @test state = Brake, doty_cycle = 0.75
    test(spirit::Motor::State::Coast, 0.75F);

    /// @test state = Brake, doty_cycle = 0.2
    test(spirit::Motor::State::Brake, 0.2F);

    /// @test state = CW, doty_cycle = 0.5
    test(spirit::Motor::State::CW, 0.5F);

    /// @test state = CCW, doty_cycle = 1.0
    test(spirit::Motor::State::CCW, 1.0F);

    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);
}

}  // namespace
