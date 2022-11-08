#include <gtest/gtest.h>

#include "MdLed.h"
#include "interfaceLeds.h"

using namespace spirit;

class testLeds : public interfaceLeds {
public:
    void write(const uint32_t value) override
    {
        _value = value;
    }

    uint32_t read() const override
    {
        return _value;
    }

private:
    uint32_t _value;
};

/**
 * @brief 初期表示のLEDが以下になることの確認
 * - Mode = Normal
 * - State = Free
 */
TEST(MdLed, InitValueTest)
{
    MdLed    mdled(bus);
    ASSERT_EQ(MdLed::Mode::Normal, mdled.mode());
    ASSERT_EQ(State::Brake, mdled.state());
}

/**
 * @brief Normalモードで設定したLEDの値が設定されているかの確認
 */
TEST(MdLed, SetGetValueTest)
{
    testLeds leds;
    MdLed    mdled(leds);

    State states[] = {State::Free, State::Brake, State::CW,    State::CCW,
                      State::CW,   State::Free,  State::Brake, State::Brake};

    // state()でSet, Get
    //  非operator
    for (State value : states) {
        mdled.state(value);
        EXPECT_EQ(MdLed::Mode::Normal, mdled.mode());
        EXPECT_EQ(value, mdled.state());
    }

    //  operator
    for (State value : states) {
        mdled = value;

        MdLed::Mode mode = mdled;
        EXPECT_EQ(MdLed::Mode::Normal, mode);

        State state = mdled;
        EXPECT_EQ(value, state);
    }

    // write(), read()でSet, Get
    //  非operator
    uint32_t values[] = {
        1, 3, 2, 0, 3, 3, 1, 0, 2, 2,
    };
    for (uint32_t value : values) {
        mdled.write(value);
        uint32_t state = mdled.read();
        EXPECT_EQ(value, state);
    }

    //  operator
    for (uint32_t value : values) {
        mdled          = value;
        uint32_t state = mdled;
        EXPECT_EQ(value, state);
    }
}

uint32_t next_led(MdLed& mdled, const uint32_t blinking_rate)
{
    // blinking_rate 期間中、値が変化していないことを確認する
    uint32_t value = mdled.read();
    for (auto i = 0; i < blinking_rate; i++) {
        EXPECT_EQ(mdled.read(), value);
        mdled.coordinate();
    }
    return mdled.read();
}

/**
 * @brief Errorモードが意図した動作をしているかの確認
 */
TEST(MdLed, ErrorBlinkTest)
{
    const uint32_t blinking_rate = 5;
    uint32_t       loop_count    = 10;
    testLeds       leds;
    MdLed          mdled(leds);
    mdled.blinking_rate(blinking_rate);

    // Error Code: 0
    mdled.error(0);
    EXPECT_EQ(mdled.read(), 3);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    for (auto i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        EXPECT_EQ(next_led(mdled, blinking_rate), 3);
    }

    // Error Code: 1
    mdled.error(1);
    EXPECT_EQ(mdled.read(), 3);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    for (auto i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 3);
    }

    // Error Code: 6
    mdled.error(6);
    EXPECT_EQ(mdled.read(), 3);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    for (auto i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 3);
    }

    // Error Code: 41
    mdled.error(41);
    EXPECT_EQ(mdled.read(), 3);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    for (auto i = 1; i < loop_count; i++) {
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        EXPECT_EQ(next_led(mdled, blinking_rate), 3);
    }
}

TEST(MdLed, AlternateBlinkTest)
{
    // loop回数などは適当
    uint32_t blinking_rate = 3;
    uint32_t loop_count    = 5;

    testLeds leds;
    MdLed    mdled(leds);

    mdled.write(0);
    mdled.blinking_rate(blinking_rate);
    mdled.mode(MdLed::Mode::Alternate);
    EXPECT_EQ(mdled.read(), 1);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Alternate);
    for (auto i = 1; i < loop_count; i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(next_led(mdled, blinking_rate), 1);
        } else {
            EXPECT_EQ(next_led(mdled, blinking_rate), 2);
        }
    }
}

TEST(MdLed, ConcurrentBlinkTest)
{
    // loop回数などは適当
    uint32_t blinking_rate = 11;
    uint32_t loop_count    = 9;

    testLeds leds;
    MdLed    mdled(leds);

    mdled.write(0);
    mdled.blinking_rate(blinking_rate);
    mdled.mode(MdLed::Mode::Concurrent);

    EXPECT_EQ(mdled.read(), 3);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Concurrent);
    for (auto i = 1; i < loop_count; i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(next_led(mdled, blinking_rate), 3);
        } else {
            EXPECT_EQ(next_led(mdled, blinking_rate), 0);
        }
    }
}

/**
 * @brief errorを設定後、reset_error()をするとErrorモードから、Normalモードになっていることの確認
 */
TEST(MdLed, ResetErrorTest)
{
    testLeds leds;
    MdLed    mdled(leds);

    EXPECT_NE(mdled.mode(), MdLed::Mode::Error);

    mdled.error(12);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);

    mdled.reset_error();
    EXPECT_NE(mdled.mode(), MdLed::Mode::Error);
}

/**
 * @brief
 * errorを設定後、stateを設定してもLEDがstateで設定したものに変化しないことの確認
 *
 * また、reset_error後にLEDが設定値になることの確認
 */
TEST(MdLed, ErrorLedTest)
{
    testLeds leds;
    MdLed    mdled(leds);

    mdled.error(12);
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    EXPECT_EQ(mdled.read(), 3);

    mdled.state(State::CCW);  // 2
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Error);
    EXPECT_NE(mdled.read(), static_cast<uint32_t>(State::CCW));
    EXPECT_EQ(mdled.state(), State::CCW);

    mdled.reset_error();
    EXPECT_EQ(mdled.mode(), MdLed::Mode::Normal);
    EXPECT_EQ(mdled.read(), static_cast<uint32_t>(State::CCW));
    EXPECT_EQ(mdled.state(), State::CCW);
}
