#include <gtest/gtest.h>

#include "MdLed.h"
#include "tests/stubs.h"

using namespace spirit;

/**
 * @brief led0, led1 の値を2bitとしてまとめた値を返す
 * @param led0 下位ビットのLED
 * @param led1 上位ビットのLED
 * @return led0, led1 の値を2bitとしてまとめた値
 */
static uint32_t leds_value(const StubDigitalOut& led0, const StubDigitalOut& led1)
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
static uint32_t compare_leds(MdLed& mdled, const StubDigitalOut& led0, const StubDigitalOut& led1, const uint32_t value,
                      uint32_t loop)
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
 * @brief State型からuint32_tに変換した値を返す
 * @param state モーターの回転方向
 * @return State型からuint32_tに変換した値
 */
static uint32_t state2uint(State state)
{
    switch (state) {
        case State::Coast:
            return 0;
        case State::CW:
            return 1;
        case State::CCW:
            return 2;
        case State::Brake:
            return 3;
        default:
            // 未定義の値
            return UINT32_MAX;
    }
}

/**
 * @brief 初期表示のLEDが以下になることの確認
 * - BlinkMode = Normal
 * - State = Brake
 */
TEST(MdLed, InitValueTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);
    mdled.blinking_rate(5);

    // blinking_rate で設定した値より多くループさせる
    uint32_t loop = 5 * 5;
    ASSERT_TRUE(compare_leds(mdled, led0, led1, state2uint(State::Brake), loop));
}

/**
 * @brief Normalモードで設定したLEDの値が設定されているかの確認
 */
TEST(MdLed, SetGetValueTest)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    State states[] = {State::Coast, State::Brake, State::CW,    State::CCW,
                      State::CW,    State::Coast, State::Brake, State::Brake};

    uint32_t rate = 80;
    mdled.blinking_rate(rate);

    // state()でSet, Get
    //  非operator
    for (State value : states) {
        mdled.state(value);
        EXPECT_TRUE(compare_leds(mdled, led0, led1, state2uint(value), rate * 5));
    }

    //  operator
    for (State value : states) {
        mdled = value;
        EXPECT_TRUE(compare_leds(mdled, led0, led1, state2uint(value), rate * 5));
    }
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
static uint32_t next_leds(MdLed& mdled, const StubDigitalOut& led0, const StubDigitalOut& led1, const uint32_t blinking_rate)
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

    // Error Code: 0
    mdled.error(0);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 1);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }

    // Error Code: 1
    mdled.error(1);
    EXPECT_EQ(leds_value(led0, led1), 3);
    for (uint32_t i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 2);
        EXPECT_EQ(next_leds(mdled, led0, led1, blinking_rate), 3);
    }

    // Error Code: 6
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

    // Error Code: 41
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
    mdled.state(State::Coast);
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
    mdled.state(State::Coast);
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

    mdled.state(State::CCW);
    EXPECT_EQ(leds_value(led0, led1), state2uint(State::CCW));

    mdled.error(12);
    EXPECT_NE(leds_value(led0, led1), state2uint(State::CCW));

    mdled.reset_error();
    EXPECT_EQ(leds_value(led0, led1), state2uint(State::CCW));
}
