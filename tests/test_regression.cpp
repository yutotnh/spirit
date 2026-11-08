#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <functional>
#include <limits>

#include "spirit.h"
#include "tests/stubs.h"

namespace {

using namespace spirit;

/// 一定速度で回している間、rps は最初の周期から一定であることの確認
TEST(Regression, SpeedControllerRpsIsStableAfterReset)
{
    StubInterruptIn a;
    StubInterruptIn b;
    SpeedController controller(a, b);
    controller.limit(1000.0F, -1000.0F);

    // 1周期に 8 カウント(2回転分の4逓倍パルス)、1周期 0.01s なので 1.0 rps
    for (int i = 0; i < 40; i++) {
        for (int pulse = 0; pulse < 2; pulse++) {
            a.interrupt_rise();
            b.interrupt_rise();
            a.interrupt_fall();
            b.interrupt_fall();
        }
        controller.calculation(1.0F, 0.01F);
        EXPECT_NEAR(controller.rps(), 1.0F, 1e-4F) << "cycle " << i;
    }
}

/// 積分項が飽和した状態から目標を超えて回り始めたとき、蓄積した誤差が出力の上限分までに制限されていて、素早く戻ることの確認
TEST(Regression, SpeedControllerAntiWindup)
{
    StubInterruptIn a;
    StubInterruptIn b;
    SpeedController controller(a, b);
    controller.limit(1.0F, -1.0F);
    controller.pid_gain(0.3F, 0.8F, 0.0F);

    // 回らないまま長時間、目標を与え続ける(積分項は飽和する)
    for (int i = 0; i < 3000; i++) {
        controller.calculation(1.0F, 0.01F);
    }

    // 目標(1 rps)の1.5倍で回す(1周期に 12 カウント)
    int cycles_until_low_output = -1;
    for (int i = 0; i < 1000; i++) {
        for (int pulse = 0; pulse < 3; pulse++) {
            a.interrupt_rise();
            b.interrupt_rise();
            a.interrupt_fall();
            b.interrupt_fall();
        }
        if (controller.calculation(1.0F, 0.01F) < 0.5F) {
            cycles_until_low_output = i;
            break;
        }
    }
    EXPECT_GE(cycles_until_low_output, 0);
    EXPECT_LT(cycles_until_low_output, 400);
}

/// モードを Normal に戻してもエラーにならず、モーターの状態が表示されることの確認
TEST(Regression, MdLedModeNormalIsValid)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    Error::get_instance().reset();
    mdled.state(Motor::State::CCW);
    mdled.mode(MdLed::BlinkMode::Alternate);
    mdled.mode(MdLed::BlinkMode::Normal);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal);
    EXPECT_EQ((led1.read() << 1) + led0.read(), 2U);
}

/// Alternate/Concurrent 中に state() を呼んでも、点滅が Normal に戻らないことの確認
TEST(Regression, MdLedStateDoesNotCancelBlinkMode)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);
    mdled.blinking_rate(1);

    mdled.mode(MdLed::BlinkMode::Alternate);
    mdled.state(Motor::State::CW);
    const uint32_t first = (led1.read() << 1) + led0.read();
    mdled.coordinate();
    const uint32_t second = (led1.read() << 1) + led0.read();
    EXPECT_NE(first, second);

    // Normal に戻すと、最後に設定された state が表示される
    mdled.mode(MdLed::BlinkMode::Normal);
    EXPECT_EQ((led1.read() << 1) + led0.read(), 1U);
}

/// 不正な state は保持されないことの確認
TEST(Regression, MdLedInvalidStateIsNotStored)
{
    StubDigitalOut led0;
    StubDigitalOut led1;
    MdLed          mdled(led0, led1);

    testing::internal::CaptureStderr();
    mdled.state(Motor::State::CW);
    mdled.state(static_cast<Motor::State>(9));
    testing::internal::GetCapturedStderr();

    mdled.mode(MdLed::BlinkMode::Alternate);
    mdled.mode(MdLed::BlinkMode::Normal);
    EXPECT_EQ((led1.read() << 1) + led0.read(), 1U);
}

/// 極小の値は符号を保ったまま 0 になることの確認
TEST(Regression, Bfloat16TinyValuesFlushToZero)
{
    for (const float value : {1e-40F, 1e-39F, 5e-39F}) {
        EXPECT_EQ(bfloat16_to_float32(float32_to_bfloat16(value)), 0.0F);
        const float negative = bfloat16_to_float32(float32_to_bfloat16(-value));
        EXPECT_EQ(negative, 0.0F);
        EXPECT_TRUE(std::signbit(negative));
    }
    // 正規化数の最小値は保持される
    EXPECT_NEAR(bfloat16_to_float32(float32_to_bfloat16(1.17549435e-38F)), 1.17549435e-38F, 1e-40F);
}

/// NaN は範囲外の値として拒否され、設定が変更されないことの確認
TEST(Regression, MotorRejectsNaN)
{
    const float nan   = std::numeric_limits<float>::quiet_NaN();
    Error&      error = Error::get_instance();

    testing::internal::CaptureStderr();

    auto check = [&](const char* name, const std::function<void(Motor&)>& set_nan) {
        Motor motor;
        error.reset();
        set_nan(motor);
        EXPECT_EQ(error.get_status(), Error::Status::Error) << name;
        EXPECT_FALSE(std::isnan(motor.get_duty_cycle())) << name;
        EXPECT_FALSE(std::isnan(motor.get_speed())) << name;
        EXPECT_FALSE(std::isnan(motor.get_pulse_period())) << name;
        EXPECT_FALSE(std::isnan(motor.get_release_time())) << name;
        float kp, ki, kd;
        motor.get_pid_gain_factor(kp, ki, kd);
        EXPECT_FALSE(std::isnan(kp) || std::isnan(ki) || std::isnan(kd)) << name;
    };

    const float inf = std::numeric_limits<float>::infinity();
    // 無限大は、通信で送れない(受信側が拒否する)ので、設定できない
    check("speed(inf)", [&](Motor& m) { m.speed(inf); });
    check("kp(inf)", [&](Motor& m) { m.pid_gain_factor(inf, 0.0F, 0.0F); });
    check("ki(inf)", [&](Motor& m) { m.pid_gain_factor(0.0F, inf, 0.0F); });
    check("kd(inf)", [&](Motor& m) { m.pid_gain_factor(0.0F, 0.0F, inf); });

    check("duty_cycle", [&](Motor& m) { m.duty_cycle(nan); });
    check("speed", [&](Motor& m) { m.speed(nan); });
    check("kp", [&](Motor& m) { m.pid_gain_factor(nan, 0.0F, 0.0F); });
    check("ki", [&](Motor& m) { m.pid_gain_factor(0.0F, nan, 0.0F); });
    check("kd", [&](Motor& m) { m.pid_gain_factor(0.0F, 0.0F, nan); });
    check("pulse_period", [&](Motor& m) { m.pulse_period(nan); });
    check("release_time", [&](Motor& m) { m.release_time(nan); });

    testing::internal::GetCapturedStderr();
}

/// A3921 と SpeedController も NaN を拒否することの確認
TEST(Regression, A3921AndSpeedControllerRejectNaN)
{
    const float nan   = std::numeric_limits<float>::quiet_NaN();
    Error&      error = Error::get_instance();

    testing::internal::CaptureStderr();

    StubDigitalOut sr, reset;
    StubPwmOut     pwmh, pwml, phase;
    A3921          a3921(sr, pwmh, pwml, phase, reset);
    a3921.state(Motor::State::CW);
    a3921.duty_cycle(0.5F);
    error.reset();
    a3921.duty_cycle(nan);
    EXPECT_NE(error.get_status(), Error::Status::Normal);
    a3921.run();
    EXPECT_FLOAT_EQ(pwml.read(), 0.5F);  // Slow decay・Low side の PWM は直前の有効な値のまま

    StubInterruptIn a, b;
    SpeedController controller(a, b);
    controller.limit(1.0F, -1.0F);
    error.reset();
    EXPECT_FLOAT_EQ(controller.calculation(nan, 0.01F), 0.0F);
    EXPECT_EQ(error.get_status(), Error::Status::Error);
    error.reset();
    EXPECT_FLOAT_EQ(controller.calculation(1.0F, nan), 0.0F);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

/// 目標値を急に変えても、微分項が(目標値の変化で)跳ね上がらないことの確認
TEST(Regression, SpeedControllerNoDerivativeKick)
{
    StubInterruptIn a;
    StubInterruptIn b;
    SpeedController controller(a, b);
    controller.limit(1000.0F, -1000.0F);
    controller.pid_gain(0.0F, 0.0F, 1.0F);

    // 回転していない状態で目標が 0 -> 10 に変わっても、微分項は 0
    EXPECT_FLOAT_EQ(controller.calculation(0.0F, 0.01F), 0.0F);
    EXPECT_FLOAT_EQ(controller.calculation(10.0F, 0.01F), 0.0F);
    EXPECT_FLOAT_EQ(controller.calculation(10.0F, 0.01F), 0.0F);

    // 速度が上がると、それを打ち消す向き(負)の出力になる
    // 1周期に 8 カウント = 1 rps (1 周期目の rps は 1.0)
    for (int pulse = 0; pulse < 2; pulse++) {
        a.interrupt_rise();
        b.interrupt_rise();
        a.interrupt_fall();
        b.interrupt_fall();
    }
    EXPECT_LT(controller.calculation(10.0F, 0.01F), 0.0F);
}

/// 設定関数が、設定できたかどうかを返すことの確認
TEST(Regression, SettersReturnWhetherAccepted)
{
    testing::internal::CaptureStderr();

    Motor motor;
    EXPECT_TRUE(motor.speed(1.0F));
    EXPECT_FALSE(motor.speed(-1.0F));
    EXPECT_FLOAT_EQ(motor.get_speed(), 1.0F);
    EXPECT_TRUE(motor.duty_cycle(0.5F));
    EXPECT_TRUE(motor.duty_cycle(2.0F));  // 範囲を超えた場合は、端に収めて(警告を通知して)設定する
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 1.0F);
    EXPECT_FALSE(motor.duty_cycle(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_TRUE(motor.pid_gain_factor(1.0F, 2.0F, 3.0F));
    EXPECT_FALSE(motor.pid_gain_factor(1.0F, -2.0F, 3.0F));
    EXPECT_TRUE(motor.state(Motor::State::CW));
    EXPECT_FALSE(motor.state(static_cast<Motor::State>(9)));
    EXPECT_EQ(motor.get_state(), Motor::State::CW);
    EXPECT_TRUE(motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::Low));
    EXPECT_FALSE(motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::Manual));
    EXPECT_TRUE(motor.full_scale_time(Motor::ChangeLevelTarget::Fall, 1.0F));
    EXPECT_FALSE(motor.full_scale_time(Motor::ChangeLevelTarget::Fall, -1.0F));
    EXPECT_TRUE(motor.pulse_period(1.0F / 1000.0F));
    EXPECT_FALSE(motor.pulse_period(1.0F));
    EXPECT_TRUE(motor.release_time(1.0F));
    EXPECT_FALSE(motor.release_time(-1.0F));
    EXPECT_TRUE(motor.decay(Motor::Decay::Fast));
    EXPECT_FALSE(motor.decay(Motor::Decay::Mixed));
    EXPECT_TRUE(motor.pwm_side(Motor::PwmSide::High));
    EXPECT_FALSE(motor.pwm_side(static_cast<Motor::PwmSide>(5)));
    EXPECT_TRUE(motor.control_system(Motor::ControlSystem::Speed));
    EXPECT_FALSE(motor.control_system(static_cast<Motor::ControlSystem>(7)));

    StubDigitalOut led0, led1;
    MdLed          led(led0, led1);
    EXPECT_TRUE(led.state(Motor::State::CW));
    EXPECT_FALSE(led.state(static_cast<Motor::State>(9)));
    EXPECT_TRUE(led.mode(MdLed::BlinkMode::Alternate));
    EXPECT_TRUE(led.mode(MdLed::BlinkMode::Alternate));  // 既に同じモード
    EXPECT_FALSE(led.mode(MdLed::BlinkMode::Error));

    StubDigitalOut sr, reset;
    StubPwmOut     pwmh, pwml, phase;
    A3921          a3921(sr, pwmh, pwml, phase, reset);
    EXPECT_TRUE(a3921.duty_cycle(0.5F));
    EXPECT_FALSE(a3921.duty_cycle(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_TRUE(a3921.state(Motor::State::CW));
    EXPECT_FALSE(a3921.state(static_cast<Motor::State>(9)));
    EXPECT_TRUE(a3921.decay(Motor::Decay::Fast));
    EXPECT_FALSE(a3921.decay(Motor::Decay::Mixed));
    EXPECT_TRUE(a3921.pwm_side(Motor::PwmSide::High));
    EXPECT_FALSE(a3921.pwm_side(static_cast<Motor::PwmSide>(5)));
    EXPECT_TRUE(a3921.pulse_period(1.0F / 1000.0F));
    EXPECT_FALSE(a3921.pulse_period(1.0F));

    StubInterruptIn a, b;
    SpeedController controller(a, b);
    EXPECT_TRUE(controller.limit(1.0F, -1.0F));
    EXPECT_FALSE(controller.limit(-1.0F, 1.0F));

    testing::internal::GetCapturedStderr();
}

}  // namespace
