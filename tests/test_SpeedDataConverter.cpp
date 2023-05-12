#include <gtest/gtest.h>

#include <cmath>

#include "SpeedDataConverter.h"

namespace {

using namespace spirit;

/**
 * @brief SpeedDataConverter::encode() と SpeedDataConverter::decode() 正常系のテスト
 */
TEST(SpeedDataConverter, EncodeDecodeSpeedTest)
{
    auto test = [](float speed, float Kp, float Ki, Motor::State state) {
        SpeedDataConverter speed_data_converter;
        Motor              motor;
        motor.speed(speed);
        motor.pid_gain_factor(Kp, Ki, 0.0F);
        motor.state(state);

        constexpr std::size_t max_buffer_size = 56;
        uint8_t               buffer[max_buffer_size / 8]{};
        std::size_t           buffer_size = 0;

        speed_data_converter.encode(motor, max_buffer_size, buffer, buffer_size);

        // 2(ヘッダ) + 16(スピード(rps)) + 16(Kp) + 16(Ki) + 2(回転方向) = 52
        constexpr std::size_t expected_buffer_size = 52;
        EXPECT_EQ(expected_buffer_size, buffer_size);

        Motor decoded_motor;
        speed_data_converter.decode(buffer, max_buffer_size, decoded_motor);

        // スピード, Kp, Kiは送受信中にbfloat16に変換している
        // bfloat16の仮数部は7bitなので、その分の誤差を考慮する必要がある
        auto allowable_error_margin = [](const float bfloat16) { return bfloat16 / powf(2.0F, 7.0F); };

        EXPECT_NEAR(speed, decoded_motor.get_speed(), allowable_error_margin(speed));

        float return_Kp = 0.0F;
        float return_Ki = 0.0F;
        float return_Kd = 0.0F;
        decoded_motor.get_pid_gain_factor(return_Kp, return_Ki, return_Kd);
        EXPECT_NEAR(Kp, return_Kp, allowable_error_margin(Kp));
        EXPECT_NEAR(Ki, return_Ki, allowable_error_margin(Ki));

        EXPECT_EQ(state, decoded_motor.get_state());
    };

    // スピードが0未満や1より大きい場合のテストはMotorクラスのテストで行う
    // とりあえず、スピードが最小/中間/適当な細かい値/最大と、各回転方向のテストを行う
    test(0.00F, 0.00F, 0.00F, Motor::State::Coast);
    test(0.00F, 0.00F, 0.00F, Motor::State::CW);
    test(0.00F, 0.00F, 0.00F, Motor::State::CCW);
    test(0.00F, 0.00F, 0.00F, Motor::State::Brake);

    test(0.50F, 0.00F, 0.00F, Motor::State::Coast);
    test(0.50F, 0.00F, 0.00F, Motor::State::Coast);
    test(0.50F, 0.00F, 0.00F, Motor::State::Coast);
    test(0.50F, 0.00F, 0.00F, Motor::State::Coast);

    test(0.00F, 0.50F, 0.00F, Motor::State::Coast);
    test(0.00F, 0.50F, 0.00F, Motor::State::Coast);
    test(0.00F, 0.50F, 0.00F, Motor::State::Coast);
    test(0.00F, 0.50F, 0.00F, Motor::State::Coast);

    test(0.00F, 0.00F, 0.50F, Motor::State::Coast);
    test(0.00F, 0.00F, 0.50F, Motor::State::Coast);
    test(0.00F, 0.00F, 0.50F, Motor::State::Coast);
    test(0.00F, 0.00F, 0.50F, Motor::State::Coast);

    test(0.50F, 0.05F, 1.00F, Motor::State::CW);
    test(1.00F, 0.75F, 0.20F, Motor::State::CCW);
}

/**
 * @brief decode時、バッファーのヘッダーから処理対象のデータかどうかを判断しているかのテスト
 */
TEST(SpeedDataConverter, HeaderCheckTest)
{
    // 正常な場合
    SpeedDataConverter    speed_data_converter;
    Motor                 motor;
    constexpr std::size_t buffer_size = 64;
    uint8_t               buffer[buffer_size / 8]{0x40};

    EXPECT_TRUE(speed_data_converter.decode(buffer, buffer_size, motor));

    /// @test 添え字0のヘッダ部分以外を見ていないことを確認するために、ヘッダ部分以外は全て1で確かめる
    buffer[0] = 0x40 | 0x3F;
    EXPECT_TRUE(speed_data_converter.decode(buffer, buffer_size, motor));

    // 異常な場合
    /// @test ヘッダ部分のFAILとなる全パターンでテスト
    buffer[0] = 0x00;
    EXPECT_FALSE(speed_data_converter.decode(buffer, buffer_size, motor));
    buffer[0] = 0x80;
    EXPECT_FALSE(speed_data_converter.decode(buffer, buffer_size, motor));
    buffer[0] = 0xC0;
    EXPECT_FALSE(speed_data_converter.decode(buffer, buffer_size, motor));
}

}  // namespace
