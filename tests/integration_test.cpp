#include <gtest/gtest.h>

#include "spirit.h"

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
    auto controller = [dip_sw](const spirit::Motor &motor) {
        spirit::FakeUdpConverter   fake_udp;
        spirit::MotorDataConverter motor_data;
        const uint32_t             can_id = spirit::can::get_motor_id(1, 0, dip_sw);

        constexpr std::size_t max_motor_data_buffer_size = 64;
        uint8_t               motor_data_buffer[8]       = {};
        std::size_t           motor_data_buffer_size;

        constexpr std::size_t max_fake_udp_buffer_size = 64;
        uint8_t               fake_udp_buffer[8]       = {};
        std::size_t           fake_udp_buffer_size;

        motor_data.encode(motor, max_motor_data_buffer_size, motor_data_buffer, motor_data_buffer_size);

        fake_udp.encode(motor_data_buffer, motor_data_buffer_size, max_fake_udp_buffer_size, fake_udp_buffer,
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
    auto peripheral = [dip_sw](const ::CANMessage &message) {
        spirit::FakeUdpConverter   fake_udp;
        spirit::MotorDataConverter motor_data;

        const uint32_t can_id = spirit::can::get_motor_id(1, 0, dip_sw);

        EXPECT_EQ(message.id, can_id);

        constexpr std::size_t max_payload_size = 64;
        uint8_t               payload[8]       = {};
        std::size_t           payload_size;
        fake_udp.decode(message.data, message.len * 8, max_payload_size, payload, payload_size);

        spirit::Motor motor;
        motor_data.decode(payload, payload_size, motor);

        return motor;
    };

    /// PWM制御
    auto pwm_test = [&controller, &peripheral](const spirit::Motor::State state, const float duty_cycle) {
        spirit::Motor motor;
        motor.control_system(spirit::Motor::ControlSystem::PWM);
        motor.duty_cycle(duty_cycle);
        motor.state(state);

        ::CANMessage  message        = controller(motor);
        spirit::Motor received_motor = peripheral(message);

        // デューティー比は16bitで表現しているため、1/65535の誤差を許容する
        constexpr float allowable_error_margin = 1.0F / 65535.0F;
        EXPECT_EQ(spirit::Motor::ControlSystem::PWM, received_motor.get_control_system());
        EXPECT_EQ(state, received_motor.get_state());
        EXPECT_NEAR(duty_cycle, received_motor.get_duty_cycle(), allowable_error_margin);
    };

    auto speed_test = [&controller, &peripheral](const spirit::Motor::State state, const float speed, const float kp,
                                                 const float ki, const float kd) {
        spirit::Motor motor;
        motor.control_system(spirit::Motor::ControlSystem::Speed);
        motor.state(state);
        motor.speed(speed);
        motor.pid_gain_factor(kp, ki, kd);

        ::CANMessage  message        = controller(motor);
        spirit::Motor received_motor = peripheral(message);

        // スピード, kp, kiは送受信中にbfloat16に変換している
        // bfloat16の仮数部は7bitなので、その分の誤差を考慮する必要がある
        auto allowable_error_margin = [](const float bfloat16) { return bfloat16 / 127.0F; };

        EXPECT_EQ(spirit::Motor::ControlSystem::Speed, received_motor.get_control_system());
        EXPECT_EQ(state, received_motor.get_state());
        EXPECT_NEAR(speed, motor.get_speed(), allowable_error_margin(speed));

        float return_kp, return_ki, return_kd;
        received_motor.get_pid_gain_factor(return_kp, return_ki, return_kd);
        EXPECT_NEAR(kp, return_kp, allowable_error_margin(kp));
        EXPECT_NEAR(ki, return_ki, allowable_error_margin(ki));
    };

    spirit::Error &error = spirit::Error::get_instance();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    // PWM制御のテスト

    /// @test state = Brake, doty_cycle = 0.75
    pwm_test(spirit::Motor::State::Coast, 0.75F);

    /// @test state = Brake, doty_cycle = 0.2
    pwm_test(spirit::Motor::State::Brake, 0.2F);

    /// @test state = CW, doty_cycle = 0.5
    pwm_test(spirit::Motor::State::CW, 0.5F);

    /// @test state = CCW, doty_cycle = 1.0
    pwm_test(spirit::Motor::State::CCW, 1.0F);

    // 速度制御(通信の都合上kdの値は無視される)
    /// @test state = Brake, speed = 0.75, kp = 0.2, ki = 0.5, kd = 1.0
    speed_test(spirit::Motor::State::Coast, 0.75F, 0.2F, 0.5F, 1.0F);

    /// @test state = Brake, speed = 0.2, kp = 0.5, ki = 1.0, kd = 0.2
    speed_test(spirit::Motor::State::Brake, 0.2F, 0.5F, 1.0F, 0.2F);

    /// @test state = CW, speed = 0.5, kp = 1.0, ki = 0.2, kd = 0.5
    speed_test(spirit::Motor::State::CW, 0.5F, 1.0F, 0.2F, 0.5F);

    /// @test state = CCW, speed = 1.0, kp = 0.2, ki = 0.5, kd = 1.0
    speed_test(spirit::Motor::State::CCW, 1.0F, 0.2F, 0.5F, 1.0F);

    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);
}

}  // namespace
