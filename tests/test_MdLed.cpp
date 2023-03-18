#include <gtest/gtest.h>

#include "MdLed.h"
#include "include/Error.h"
#include "tests/stubs.h"

namespace {

using namespace spirit;

/**
 * @brief led0, led1 の値を2bitとしてまとめた値を返す
 * @param led0 下位ビットのLED
 * @param led1 上位ビットのLED
 * @return led0, led1 の値を2bitとしてまとめた値
 */
uint32_t leds_value(StubDigitalOut& led0, StubDigitalOut& led1)
{
    return (led1.read() << 1) + led0.read();
}

/**
 * @brief led0, led1 の値を2bitとしてまとめた値と value を比較する
 * @param mdled 対象のLED
 * @param led0 下位ビットのLED
 * @param led1 上位ビットのLED
 * @param value 繰り返し数える回数
 * @param loop 繰り返し数える回数
 * @retval true 同じ
 * @retval false 異なる
 * @note BlinkMode::Normal では MdLed::blinking_rate() で指定した回数より多く同じ値にならないので @n
 * loop があることで BlinkMode::Normal かそれ以外かの判別がつく
 */
uint32_t compare_leds(MdLed& mdled, StubDigitalOut& led0, StubDigitalOut& led1, const uint32_t value, uint32_t loop)
{
    for (uint32_t i = 0; i < loop; i++) {
        if (value != leds_value(led0, led1)) {
            return false;
        }
        mdled.coordinate();
    }
    return true;
}

/**
 * @brief Motor::State型からuint32_tに変換した値を返す
 * @param state モーターの回転方向
 * @return Motor::State型からuint32_tに変換した値
 */
uint32_t state2uint(Motor::State state)
{
    switch (state) {
        case Motor::State::Coast:
            return 0;
        case Motor::State::CW:
            return 1;
        case Motor::State::CCW:
            return 2;
        case Motor::State::Brake:
            return 3;
        default:
            // 未定義の値
            return UINT32_MAX;
    }
}

/**
 * @brief 初期表示のLEDが以下になることの確認
 * - BlinkMode = Normal
 * - Motor::State = Brake
 */
TEST(MdLed, InitValueTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);
    mdled.blinking_rate(5);

    // blinking_rate で設定した値より多くループさせる
    uint32_t loop = 5 * 5;
    ASSERT_TRUE(compare_leds(mdled, led0, led1, state2uint(Motor::State::Brake), loop));
}

/**
 * @brief Normalモードで設定したLEDの値が設定されているかの確認
 */
TEST(MdLed, SetGetValueTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    Motor::State states[] = {Motor::State::Coast, Motor::State::Brake, Motor::State::CW,    Motor::State::CCW,
                             Motor::State::CW,    Motor::State::Coast, Motor::State::Brake, Motor::State::Brake};

    uint32_t rate = 80;
    mdled.blinking_rate(rate);

    for (Motor::State state : states) {
        mdled.state(state);
        EXPECT_TRUE(compare_leds(mdled, led0, led1, state2uint(state), rate * 5));
    }
}

/**
 * @brief 未定義の Mode が設定された場合にエラーになることの確認
 */
TEST(MdLed, UndefinedModeTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    // 未定義の値を設定
    Error& error = Error::get_instance();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    mdled.mode(static_cast<MdLed::BlinkMode>(4));
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    error.reset();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    mdled.mode(MdLed::BlinkMode::Error);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

/**
 * @brief 未定義の State が設定された場合にエラーになることの確認
 */
TEST(MdLed, UndefinedStateTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    // 未定義の値を設定
    Error& error = Error::get_instance();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    mdled.state(static_cast<Motor::State>(4));
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

/**
 * @brief 次の周期のLEDの値を返す @n
 * 同じ期間中にLEDの値が変化していないことのテストも行う
 * @param mdled 対象のLED
 * @param led0 下位ビットのLED
 * @param led1 上位ビットのLED
 * @param blinking_rate
 * @return 次の周期のLEDの値
 */
uint32_t next_leds(MdLed& mdled, StubDigitalOut& led0, StubDigitalOut& led1, const uint32_t blinking_rate)
{
    // blinking_rate 期間中、値が変化していないことを確認する
    uint32_t value = leds_value(led0, led1);

    for (uint32_t i = 0; i < blinking_rate; i++) {
        EXPECT_EQ(leds_value(led0, led1), value);
        mdled.coordinate();
    }

    return leds_value(led0, led1);
}

/**
 * @brief Errorモードが意図した動作をしているかの確認
 * @details ビット幅の異なるいくつかの値について確認する
 */
TEST(MdLed, ErrorBlinkTest)
{
    const uint32_t blinking_rate = 5;
    uint32_t       loop_count    = 10;
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);
    mdled.blinking_rate(blinking_rate);

    /// @test Error Code: 0
    mdled.error(0);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }

    /// @test Error Code: 1
    mdled.error(1);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }

    /// @test Error Code: 6
    mdled.error(6);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }

    /// @test Error Code: 41
    mdled.error(41);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }
}

/**
 * @brief MdLed::BlinkMode::Alternate のテスト
 */
TEST(MdLed, AlternateBlinkTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    // 点滅の開始時の値を固定化するためにLEDの値を0にする必要があるので、
    // 一旦Normal & Coast にする
    mdled.mode(MdLed::BlinkMode::Normal);
    mdled.state(Motor::State::Coast);
    EXPECT_EQ(leds_value(led0, led1), 0);

    // loop回数などは適当
    uint32_t blinking_rate = 3;
    uint32_t loop_count    = 5;
    mdled.blinking_rate(blinking_rate);
    mdled.mode(MdLed::BlinkMode::Alternate);

    EXPECT_EQ(leds_value(led0, led1), 1);

    for (uint32_t i = 1; i < loop_count; i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        } else {
            EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        }
    }
}

/**
 * @brief MdLed::BlinkMode::Concurrent のテスト
 */
TEST(MdLed, ConcurrentBlinkTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    // 点滅の開始時の値を固定化するためにLEDの値を0にする必要があるので、
    // 一旦Normal & Coast にする
    mdled.mode(MdLed::BlinkMode::Normal);
    mdled.state(Motor::State::Coast);
    EXPECT_EQ(leds_value(led0, led1), 0);

    // loop回数などは適当
    uint32_t blinking_rate = 11;
    uint32_t loop_count    = 9;
    mdled.blinking_rate(blinking_rate);
    mdled.mode(MdLed::BlinkMode::Concurrent);

    EXPECT_EQ(leds_value(led0, led1), 3);

    for (uint32_t i = 1; i < loop_count; i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
        } else {
            EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 0);
        }
    }
}

/**
 * @brief errorを設定後、reset_error()をするとErrorモードから、Normalモードになっていることの確認
 */
TEST(MdLed, ResetErrorTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    mdled.state(Motor::State::CCW);
    EXPECT_EQ(leds_value(led0, led1), state2uint(Motor::State::CCW));

    mdled.error(12);
    EXPECT_NE(leds_value(led0, led1), state2uint(Motor::State::CCW));

    mdled.reset_error();
    EXPECT_EQ(leds_value(led0, led1), state2uint(Motor::State::CCW));
}

}  // namespace
