// Arduino 用のコードを、 Arduino.h のスタブを使ってホスト上でテストする
//
// 実機の core の挙動(ピンの対応や実際の PWM 周波数など)は確認できない
// 確認するのは、このライブラリが core の API を想定した順序・値で呼んでいることと、ライブラリ側の変換ロジック
//
// ビルド構成は以下の4つ (tests/CMakeLists.txt)
// - ARDUINO のみ ... analogWrite() を使うボード
// - ARDUINO + ESP32 + ESP_ARDUINO_VERSION_MAJOR=2 ... LEDC (チャンネル指定)
// - ARDUINO + ESP32 + ESP_ARDUINO_VERSION_MAJOR=3 ... LEDC (ピン指定)

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

#include "Arduino.h"
#include "Error.h"
#include "SpeedController.h"
#include "spirit/platform/arduino/include/Delay.h"
#include "spirit/platform/arduino/include/DigitalOut.h"
#include "spirit/platform/arduino/include/InterruptIn.h"
#include "spirit/platform/arduino/include/PwmOut.h"

namespace {

using namespace spirit;

class ArduinoTest : public testing::Test {
protected:
    void SetUp() override
    {
        arduino_stub::calls().clear();
        arduino_stub::digital_read_values().clear();
        arduino_stub::interrupts().clear();
    }

    static bool called(const std::string& call)
    {
        const auto& calls = arduino_stub::calls();
        return std::find(calls.begin(), calls.end(), call) != calls.end();
    }

    static std::string last_call()
    {
        return arduino_stub::calls().empty() ? "" : arduino_stub::calls().back();
    }
};

TEST_F(ArduinoTest, DigitalOut)
{
    arduino::DigitalOut out(5);
    EXPECT_TRUE(called("pinMode(5,1,0)"));

    out.write(0);
    EXPECT_EQ(last_call(), "digitalWrite(5,0,0)");
    out.write(1);
    EXPECT_EQ(last_call(), "digitalWrite(5,1,0)");
    // 0 以外は全て HIGH
    out.write(7);
    EXPECT_EQ(last_call(), "digitalWrite(5,1,0)");

    arduino_stub::digital_read_values()[5] = HIGH;
    EXPECT_EQ(out.read(), 1U);
}

TEST_F(ArduinoTest, PwmOutDutyCycleIsClamped)
{
    arduino::PwmOut out(3, 2);

    out.write(0.5F);
    EXPECT_FLOAT_EQ(out.read(), 0.5F);

    out.write(1.5F);
    EXPECT_FLOAT_EQ(out.read(), 1.0F);

    out.write(-0.5F);
    EXPECT_FLOAT_EQ(out.read(), 0.0F);

    out.write(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FLOAT_EQ(out.read(), 0.0F);
}

TEST_F(ArduinoTest, PwmOutPeriod)
{
    arduino::PwmOut out(3, 2);
    EXPECT_FLOAT_EQ(out.read_period(), 1.0F / 5000.0F);

    out.period(1.0F / 20000.0F);
    EXPECT_FLOAT_EQ(out.read_period(), 1.0F / 20000.0F);

    // 範囲外の値は無視する
    out.period(0.0F);
    out.period(-1.0F);
    out.period(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FLOAT_EQ(out.read_period(), 1.0F / 20000.0F);
}

#ifdef ESP32

TEST_F(ArduinoTest, EspNeverUsesDac)
{
    arduino::PwmOut out(3, 2);
    out.write(1.0F);
    out.period(1.0F / 10000.0F);
    EXPECT_FALSE(called("dacWrite(3,255,0)"));
    for (const auto& call : arduino_stub::calls()) {
        EXPECT_NE(call.find("dacWrite"), 0U) << call;
        EXPECT_NE(call.find("analogWrite"), 0U) << call;
    }
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)

TEST_F(ArduinoTest, EspLedcV3)
{
    arduino::PwmOut out(3, 2);
    // 既定の周期 5kHz、分解能 10bit でピンに割り当てる
    EXPECT_TRUE(called("ledcAttach(3,5000,10)"));

    out.write(0.5F);
    EXPECT_EQ(last_call(), "ledcWrite(3,512,0)");  // 1023 * 0.5 を四捨五入
    out.write(1.0F);
    EXPECT_EQ(last_call(), "ledcWrite(3,1023,0)");
    out.write(0.0F);
    EXPECT_EQ(last_call(), "ledcWrite(3,0,0)");

    // 周期を変えると、周波数を変更して、現在のデューティー比を設定し直す
    out.write(1.0F);
    arduino_stub::calls().clear();
    out.period(1.0F / 10000.0F);
    ASSERT_EQ(arduino_stub::calls().size(), 2U);
    EXPECT_EQ(arduino_stub::calls()[0], "ledcChangeFrequency(3,10000,10)");
    EXPECT_EQ(arduino_stub::calls()[1], "ledcWrite(3,1023,0)");
}

#else

TEST_F(ArduinoTest, EspLedcV2)
{
    arduino::PwmOut out(3, 2);
    // 既定の周期 5kHz、分解能 10bit でチャンネルを設定し、ピンに割り当てる
    EXPECT_TRUE(called("ledcSetup(2,5000,10)"));
    EXPECT_TRUE(called("ledcAttachPin(3,2,0)"));

    out.write(0.5F);
    EXPECT_EQ(last_call(), "ledcWrite(2,512,0)");  // 1023 * 0.5 を四捨五入
    out.write(1.0F);
    EXPECT_EQ(last_call(), "ledcWrite(2,1023,0)");
    out.write(0.0F);
    EXPECT_EQ(last_call(), "ledcWrite(2,0,0)");

    out.write(1.0F);
    arduino_stub::calls().clear();
    out.period(1.0F / 10000.0F);
    ASSERT_EQ(arduino_stub::calls().size(), 2U);
    EXPECT_EQ(arduino_stub::calls()[0], "ledcSetup(2,10000,10)");
    EXPECT_EQ(arduino_stub::calls()[1], "ledcWrite(2,1023,0)");
}

#endif  // ESP_ARDUINO_VERSION_MAJOR

#else

TEST_F(ArduinoTest, AnalogWrite)
{
    arduino::PwmOut out(3, 2);

    out.write(0.0F);
    EXPECT_EQ(last_call(), "analogWrite(3,0,0)");
    out.write(0.5F);
    EXPECT_EQ(last_call(), "analogWrite(3,128,0)");  // 127.5 を四捨五入
    out.write(1.0F);
    EXPECT_EQ(last_call(), "analogWrite(3,255,0)");
    // 範囲外はクランプしてから変換する
    out.write(2.0F);
    EXPECT_EQ(last_call(), "analogWrite(3,255,0)");
    out.write(-1.0F);
    EXPECT_EQ(last_call(), "analogWrite(3,0,0)");

    // 周期を変えても、 analogWrite() の出力は変わらない
    arduino_stub::calls().clear();
    out.period(1.0F / 10000.0F);
    EXPECT_TRUE(arduino_stub::calls().empty());
}

#endif  // ESP32

/// 割り込みの処理の呼び出しを数える InterruptHandler
class CountingHandler : public InterruptHandler {
public:
    void on_interrupt() override
    {
        count++;
    }

    int count{0};
};

TEST_F(ArduinoTest, Delay)
{
    arduino::Delay delay;
    delay.delay_us(2);
    EXPECT_EQ(last_call(), "delayMicroseconds(2,0,0)");
}

TEST_F(ArduinoTest, InterruptInAttachesAccordingToHandlers)
{
    arduino::InterruptIn pin(2);
    CountingHandler      rise, fall;

    // 何も登録していないときは、 attachInterrupt() を呼ばない
    EXPECT_TRUE(arduino_stub::interrupts().empty());

    pin.rise(&rise);
    ASSERT_EQ(arduino_stub::interrupts().count(2), 1U);
    EXPECT_EQ(arduino_stub::interrupts()[2].mode, RISING);

    pin.fall(&fall);
    EXPECT_EQ(arduino_stub::interrupts()[2].mode, CHANGE);

    pin.rise(nullptr);
    EXPECT_EQ(arduino_stub::interrupts()[2].mode, FALLING);

    pin.fall(nullptr);
    EXPECT_TRUE(arduino_stub::interrupts().empty());
    EXPECT_TRUE(called("detachInterrupt(2,0,0)"));
}

TEST_F(ArduinoTest, InterruptInDispatchesToTheOwnHandler)
{
    arduino::InterruptIn pin_a(2);
    arduino::InterruptIn pin_b(3);
    CountingHandler      rise_a, fall_a, rise_b;

    pin_a.rise(&rise_a);
    pin_a.fall(&fall_a);
    pin_b.rise(&rise_b);

    // CHANGE のピンは、立ち上がりか立ち下がりかを、ピンの状態で判断する
    arduino_stub::digital_read_values()[2] = HIGH;
    EXPECT_TRUE(arduino_stub::fire_interrupt(2));
    arduino_stub::digital_read_values()[2] = LOW;
    EXPECT_TRUE(arduino_stub::fire_interrupt(2));
    EXPECT_TRUE(arduino_stub::fire_interrupt(3));

    EXPECT_EQ(rise_a.count, 1);
    EXPECT_EQ(fall_a.count, 1);
    EXPECT_EQ(rise_b.count, 1);  // ピン3 は、ピン2 の割り込みの影響を受けない
}

TEST_F(ArduinoTest, InterruptInReleasesSlotOnDestruction)
{
    // スロットは max_instances 個まで。破棄すると、再利用できる
    std::vector<std::unique_ptr<arduino::InterruptIn>> pins;
    for (uint8_t i = 0; i < arduino::InterruptIn::max_instances; i++) {
        pins.emplace_back(new arduino::InterruptIn(static_cast<uint8_t>(10 + i)));
    }

    testing::internal::CaptureStderr();
    Error::get_instance().reset();
    arduino::InterruptIn overflow(30);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Error);
    testing::internal::GetCapturedStderr();

    CountingHandler handler;
    overflow.rise(&handler);  // スロットがないので、何も登録されない
    EXPECT_TRUE(arduino_stub::interrupts().empty());

    pins.pop_back();
    arduino::InterruptIn reuse(31);
    Error::get_instance().reset();
    reuse.rise(&handler);
    EXPECT_EQ(arduino_stub::interrupts().count(31), 1U);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal);
}

/// Arduino のアダプターで、エンコーダーのパルスをカウントして、速度制御ができることの確認 (SpeedController が割り込みを登録する)
TEST_F(ArduinoTest, SpeedControllerCountsThroughAdapter)
{
    arduino::InterruptIn a(2);
    arduino::InterruptIn b(3);
    SpeedController      controller(a, b);
    controller.limit(1000.0F, -1000.0F);

    auto change = [](int pin, int value) {
        arduino_stub::digital_read_values()[pin] = value;
        EXPECT_TRUE(arduino_stub::fire_interrupt(pin));
    };

    // 正転(A相が先行): A↑ B↑ A↓ B↓ で 1.8° (4逓倍)
    auto forward = [&]() {
        change(2, 1);
        change(3, 1);
        change(2, 0);
        change(3, 0);
    };
    // 逆転(B相が先行)
    auto backward = [&]() {
        change(3, 1);
        change(2, 1);
        change(3, 0);
        change(2, 0);
    };

    forward();
    EXPECT_FLOAT_EQ(controller.angle(), 1.8F);
    forward();
    EXPECT_FLOAT_EQ(controller.angle(), 3.6F);
    backward();
    EXPECT_FLOAT_EQ(controller.angle(), 1.8F);

    // 1 rps (1周期 0.01s に 8 カウント) で回すと、速度が求まる
    controller.reset();
    for (int i = 0; i < 5; i++) {
        forward();
        forward();
        controller.calculation(1.0F, 0.01F);
        EXPECT_NEAR(controller.rps(), 1.0F, 1e-4F);
    }
}

}  // namespace
