// Mbed 用のアダプターを、 mbed.h のスタブを使ってホスト上でテストする
//
// 実機の Mbed (Mbed OS 6 / Mbed CE) の挙動(ピンの対応や割り込みの遅延など)は確認できない
// 確認するのは、アダプターが Mbed の API を想定した形で呼んでいることと、ライブラリ本体(SpeedController など)との組み合わせ

#include <gtest/gtest.h>

#include "Error.h"
#include "SpeedController.h"
#include "mbed.h"
#include "spirit/platform/mbed/include/Delay.h"
#include "spirit/platform/mbed/include/DigitalOut.h"
#include "spirit/platform/mbed/include/InterruptIn.h"
#include "spirit/platform/mbed/include/PwmOut.h"

namespace {

using namespace spirit;

class MbedTest : public testing::Test {
protected:
    void SetUp() override
    {
        ::mbed::pin_states().clear();
        wait_us_calls().clear();
    }
};

class CountingHandler : public InterruptHandler {
public:
    void on_interrupt() override
    {
        count++;
    }

    int count{0};
};

TEST_F(MbedTest, DigitalOut)
{
    spirit::mbed::DigitalOut out(PA_1);
    out.write(1);
    EXPECT_EQ(::mbed::pin_states()[PA_1].digital, 1);
    EXPECT_EQ(out.read(), 1U);
    out.write(0);
    EXPECT_EQ(out.read(), 0U);
}

TEST_F(MbedTest, PwmOut)
{
    spirit::mbed::PwmOut out(PA_2);
    out.write(0.25F);
    EXPECT_FLOAT_EQ(::mbed::pin_states()[PA_2].duty_cycle, 0.25F);
    EXPECT_FLOAT_EQ(out.read(), 0.25F);
    out.period(1.0F / 5000.0F);
    EXPECT_FLOAT_EQ(::mbed::pin_states()[PA_2].period, 1.0F / 5000.0F);
}

TEST_F(MbedTest, Delay)
{
    spirit::mbed::Delay delay;
    delay.delay_us(2);
    ASSERT_EQ(wait_us_calls().size(), 1U);
    EXPECT_EQ(wait_us_calls()[0], 2);

    // int に収まらない値は、収める
    delay.delay_us(0xFFFFFFFFU);
    EXPECT_EQ(wait_us_calls()[1], 0x7FFFFFFF);
}

TEST_F(MbedTest, InterruptInRegistersAndUnregistersHandlers)
{
    spirit::mbed::InterruptIn pin(PA_0);
    ::mbed::InterruptIn&      mbed_pin = *::mbed::interrupt_pins()[PA_0];
    CountingHandler           rise, fall;

    EXPECT_FALSE(mbed_pin.has_rise());
    EXPECT_FALSE(mbed_pin.has_fall());

    pin.rise(&rise);
    pin.fall(&fall);
    EXPECT_TRUE(mbed_pin.has_rise());
    EXPECT_TRUE(mbed_pin.has_fall());

    mbed_pin.fire(true);
    mbed_pin.fire(true);
    mbed_pin.fire(false);
    EXPECT_EQ(rise.count, 2);
    EXPECT_EQ(fall.count, 1);
    EXPECT_EQ(pin.read(), 0U);

    pin.rise(nullptr);
    EXPECT_FALSE(mbed_pin.has_rise());
    EXPECT_TRUE(mbed_pin.has_fall());
    pin.fall(nullptr);
    EXPECT_FALSE(mbed_pin.has_fall());
}

/// Mbed のアダプターで、エンコーダーのパルスをカウントして、速度制御ができることの確認
TEST_F(MbedTest, SpeedControllerCountsThroughAdapter)
{
    spirit::mbed::InterruptIn a(PA_0);
    spirit::mbed::InterruptIn b(PA_1);
    ::mbed::InterruptIn&      a_pin = *::mbed::interrupt_pins()[PA_0];
    ::mbed::InterruptIn&      b_pin = *::mbed::interrupt_pins()[PA_1];

    {
        SpeedController controller(a, b);
        controller.limit(1000.0F, -1000.0F);

        // 正転: A↑ B↑ A↓ B↓
        auto forward = [&]() {
            a_pin.fire(true);
            b_pin.fire(true);
            a_pin.fire(false);
            b_pin.fire(false);
        };

        forward();
        EXPECT_FLOAT_EQ(controller.angle(), 1.8F);

        // 1 rps (1周期 0.01s に 8 カウント)
        controller.reset();
        for (int i = 0; i < 5; i++) {
            forward();
            forward();
            controller.calculation(1.0F, 0.01F);
            EXPECT_NEAR(controller.rps(), 1.0F, 1e-4F);
        }

        EXPECT_TRUE(a_pin.has_rise());
    }

    // SpeedController が破棄されると、割り込みが解除される
    EXPECT_FALSE(a_pin.has_rise());
    EXPECT_FALSE(b_pin.has_fall());
}

}  // namespace
