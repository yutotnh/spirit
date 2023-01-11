#include <gtest/gtest.h>

#include "Motor.h"
#include "PwmDataConverter.h"

namespace {

using namespace spirit;

/**
 * @brief PwmDataConverter::encode() と PwmDataConverter::decode() 正常系のテスト
 */
TEST(PwmDataConverter, EncodeDecodePwmTest)
{
    auto encode_decode = [](float duty_cycle, Motor::State state) {
        PwmDataConverter pwm_data_converter;
        Motor            motor;
        motor.duty_cycle(duty_cycle);
        motor.state(state);

        constexpr std::size_t max_buffer_size = 24;
        uint8_t               buffer[max_buffer_size / 8]{};
        std::size_t           buffer_size = 0;

        pwm_data_converter.encode(motor, max_buffer_size, buffer, buffer_size);

        // 2(ヘッダ) + 16(デューティー比) + 2(回転方向) = 20
        constexpr std::size_t expected_buffer_size = 20;
        EXPECT_EQ(expected_buffer_size, buffer_size);

        Motor decoded_motor;
        pwm_data_converter.decode(buffer, max_buffer_size, decoded_motor);

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

/**
 * @brief Encode 時のバッファサイズがアクセスする領域以内か返す機能のテスト
 */
TEST(PwmDataConverter, MaxBufferSizeTest)
{
    auto max_buffer_size_test = [](std::size_t max_buffer_size, bool expected_is_succeeded) {
        PwmDataConverter pwm_data_converter;
        Motor            motor;

        // 今のところバッファサイズは8以上を想定していないので8にする
        uint8_t buffer[8]{};

        std::size_t buffer_size = 0;

        bool is_succeeded = pwm_data_converter.encode(motor, max_buffer_size, buffer, buffer_size);
        EXPECT_EQ(expected_is_succeeded, is_succeeded);
    };

    // バッファサイズが0、境界値の場合のテストを行う
    max_buffer_size_test(0, false);
    max_buffer_size_test(16, false);
    max_buffer_size_test(24, true);
    max_buffer_size_test(32, true);
}

/**
 * @brief Decode 時のバッファサイズがアクセスする領域以内か返す機能のテスト
 */
TEST(PwmDataConverter, EncodeBufferSizeTest)
{
    auto buffer_size_test = [](std::size_t encode_buffer_size, bool expected_is_succeeded) {
        PwmDataConverter pwm_data_converter;
        Motor            motor;

        constexpr std::size_t max_buffer_size = 8;
        uint8_t               buffer[max_buffer_size]{};

        Motor decoded_motor;
        bool  is_succeeded = pwm_data_converter.decode(buffer, encode_buffer_size, decoded_motor);

        EXPECT_EQ(expected_is_succeeded, is_succeeded);
    };

    // バッファサイズが0、境界値の場合のテストを行う
    buffer_size_test(0, false);
    buffer_size_test(6, false);
    buffer_size_test(24, true);
    buffer_size_test(32, true);
}

}
