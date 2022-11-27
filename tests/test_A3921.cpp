#include <gtest/gtest.h>

#include <cmath>

#include "A3921.h"
#include "tests/stubs.h"

using namespace spirit;

/**
 * @brief A3921 が Brake 状態であるかを返す
 * @param pwmh PWMH pin
 * @param pwml PWML pin
 * @retval true Brake 状態
 * @retval false 非 Brake 状態
 */
bool isBrake(const StubPwmOut& pwmh, const StubPwmOut& pwml)
{
    if (((fabsf(pwmh.read() - 1.00F) < FLT_EPSILON) && (fabsf(pwml.read() - 0.00F) < FLT_EPSILON)) ||
        ((fabsf(pwmh.read() - 0.00F) < FLT_EPSILON) && (fabsf(pwml.read() - 1.00F) < FLT_EPSILON))) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief A3921 が Coast 状態であるかを返す
 * @param pwmh PWMH pin
 * @param pwml PWML pin
 * @retval true Coast 状態
 * @retval false 非 Coast 状態
 */
bool isCoast(const StubPwmOut& pwmh, const StubPwmOut& pwml)
{
    if ((fabsf(pwmh.read() - 0.00F) < FLT_EPSILON) && (fabsf(pwml.read() - 0.00F) < FLT_EPSILON)) {
        return true;
    }

    return false;
}

/**
 * @brief 初期値のテスト
 * - sleep モード: OFF
 * - モーター: Brake
 * - パルス周期: InterfaceMotor::default_pulse_period
 */
TEST(A3921, InitValueTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    EXPECT_EQ(reset.read(), 1);
    EXPECT_FLOAT_EQ(pwmh.read_period(), InterfaceMotor::default_pulse_period);
    EXPECT_FLOAT_EQ(pwml.read_period(), InterfaceMotor::default_pulse_period);
    EXPECT_FLOAT_EQ(phase.read_period(), InterfaceMotor::default_pulse_period);

    EXPECT_TRUE(isBrake(pwmh, pwml));
}

/**
 * @brief スリープ機能のテスト
 * @details A3921 は RESET pin にLow(0) を入力することで、Sleep モードに移行する。
 */
TEST(A3921, SleepTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    a3921.sleep(true);
    EXPECT_EQ(reset.read(), 0);

    a3921.sleep(false);
    EXPECT_EQ(reset.read(), 1);
}

/**
 * @brief リセット機能のテスト(未実装のため、テストはスキップする)
 */
TEST(A3921, ResetTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    GTEST_SKIP_("Not implemented because the Reset function is not yet supported.");
    // TODO 未実装のため、実装時に行うであろうフローのイメージを記述
    // timer.start();
    // a3921.reset();
    // timer.stop();
    // EXPECT_LE(0.35us, timer.read());
}

/**
 * @brief Slow decay でのモーター制御のテスト
 * @details  spirit::A3921::duty_cycle() に0.50Fを入力することで、Coast/Brake 状態になったときに
 * pwmh, pwml が意図した出力(0.50F以外)になっていることを確認する
 */
TEST(A3921, SlowDecayTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    // 共通の設定
    a3921.decay(A3921::Decay::Slow);
    a3921.duty_cycle(0.50F);

    // Coast
    a3921.state(State::Coast);
    a3921.run();

    EXPECT_TRUE(isCoast(pwmh, pwml));

    // CW
    a3921.state(State::CW);
    a3921.run();

    EXPECT_EQ(sr.read(), 1);
    EXPECT_FLOAT_EQ(pwmh.read(), 1.00F);
    EXPECT_FLOAT_EQ(pwml.read(), 0.50F);
    EXPECT_FLOAT_EQ(phase.read(), 1.00F);

    // CCW
    a3921.state(State::CCW);
    a3921.run();

    EXPECT_EQ(sr.read(), 1);
    EXPECT_FLOAT_EQ(pwmh.read(), 1.00F);
    EXPECT_FLOAT_EQ(pwml.read(), 0.50F);
    EXPECT_FLOAT_EQ(phase.read(), 0.00F);

    // Brake
    a3921.state(State::Brake);
    a3921.run();

    EXPECT_TRUE(isBrake(pwmh, pwml));
}

/**
 * @brief Fast decay でのモーター制御のテスト
 * @details  spirit::A3921::duty_cycle() に0.50Fを入力することで、Coast/Brake 状態になったときに
 * pwmh, pwml が意図した出力(0.50F以外)になっていることを確認する
 */
TEST(A3921, FastDecayTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    // 共通の設定
    a3921.decay(A3921::Decay::Fast);
    a3921.duty_cycle(0.50F);

    // Coast
    a3921.state(State::Coast);
    a3921.run();

    EXPECT_TRUE(isCoast(pwmh, pwml));

    // CW
    a3921.state(State::CW);
    a3921.run();

    EXPECT_EQ(sr.read(), 0);
    EXPECT_FLOAT_EQ(pwmh.read(), 1.00F);
    EXPECT_FLOAT_EQ(pwml.read(), 1.00F);
    EXPECT_FLOAT_EQ(phase.read(), 0.75F);

    // CCW
    a3921.state(State::CCW);
    a3921.run();

    EXPECT_EQ(sr.read(), 0);
    EXPECT_FLOAT_EQ(pwmh.read(), 1.00F);
    EXPECT_FLOAT_EQ(pwml.read(), 1.00F);
    EXPECT_FLOAT_EQ(phase.read(), 0.25F);

    // Brake
    a3921.state(State::Brake);
    a3921.run();

    EXPECT_TRUE(isBrake(pwmh, pwml));
}

/**
 * @brief パルス周期の設定テスト
 */
TEST(A3921, PulsePeriodTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    // InterfaceMotor::default_pulse_period と異なる値を設定するため、 0.005Fを加える
    float pulse_period = InterfaceMotor::default_pulse_period + 0.005F;
    a3921.pulse_period(pulse_period);

    EXPECT_FLOAT_EQ(pwmh.read_period(), pulse_period);
    EXPECT_FLOAT_EQ(pwml.read_period(), pulse_period);
    EXPECT_FLOAT_EQ(phase.read_period(), pulse_period);
}
