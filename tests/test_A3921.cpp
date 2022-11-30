#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <thread>

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
bool is_brake(const StubPwmOut& pwmh, const StubPwmOut& pwml)
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
bool is_coast(const StubPwmOut& pwmh, const StubPwmOut& pwml)
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

    EXPECT_TRUE(is_brake(pwmh, pwml));
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
 * @brief リセット機能のテスト
 * @details テスト内容 @n
 * - A3921::reset() 前後で、 RESETピンが High になっていること
 * - A3921::reset() の実行時間が 3.5us 以上であること
 * @note 0.1us <= rest time <= 3.5us だが、(おそらく)マイコンでないとその範囲におさまらない @n
 * そのため、テストでは与えた関数が実行されて待ち時間が発生していることを検証する
 */
TEST(A3921, ResetTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    EXPECT_EQ(reset.read(), 1);

    // 前述の理由で Reset Pulse Time の測定は 100us 待つ関数を与えて、 100us <= 経過時間 になることを確認

    std::function<void(void)> sleep = []() { std::this_thread::sleep_for(std::chrono::microseconds(100)); };

    auto start = std::chrono::system_clock::now();
    a3921.reset(sleep);
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;

    EXPECT_EQ(reset.read(), 1);
    EXPECT_LE(100.0e-6, elapsed_seconds.count());
}

/**
 * @brief Slow decay low side でのモーター制御のテスト
 * @details  spirit::A3921::duty_cycle() に0.50Fを入力することで、Coast/Brake 状態になったときに
 * pwmh, pwml が意図した出力(0.50F以外)になっていることを確認する
 */
TEST(A3921, SlowDecayLowSideTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    // 共通の設定
    a3921.decay(A3921::Decay::Slow);
    a3921.pwm_side(A3921::PwmSide::Low);
    a3921.duty_cycle(0.50F);

    // Coast
    a3921.state(State::Coast);
    a3921.run();

    EXPECT_TRUE(is_coast(pwmh, pwml));

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

    EXPECT_TRUE(is_brake(pwmh, pwml));
}

/**
 * @brief Slow decay high side でのモーター制御のテスト
 * @details  spirit::A3921::duty_cycle() に0.50Fを入力することで、Coast/Brake 状態になったときに
 * pwmh, pwml が意図した出力(0.50F以外)になっていることを確認する
 */
TEST(A3921, SlowDecayHighSideTest)
{
    StubDigitalOut sr;
    StubPwmOut     pwmh;
    StubPwmOut     pwml;
    StubPwmOut     phase;
    StubDigitalOut reset;
    A3921          a3921(sr, pwmh, pwml, phase, reset);

    // 共通の設定
    a3921.decay(A3921::Decay::Slow);
    a3921.pwm_side(A3921::PwmSide::High);
    a3921.duty_cycle(0.50F);

    // Coast
    a3921.state(State::Coast);
    a3921.run();

    EXPECT_TRUE(is_coast(pwmh, pwml));

    // CW
    a3921.state(State::CW);
    a3921.run();

    EXPECT_EQ(sr.read(), 1);
    EXPECT_FLOAT_EQ(pwmh.read(), 0.50F);
    EXPECT_FLOAT_EQ(pwml.read(), 1.00F);
    EXPECT_FLOAT_EQ(phase.read(), 1.00F);

    // CCW
    a3921.state(State::CCW);
    a3921.run();

    EXPECT_EQ(sr.read(), 1);
    EXPECT_FLOAT_EQ(pwmh.read(), 0.50F);
    EXPECT_FLOAT_EQ(pwml.read(), 1.00F);
    EXPECT_FLOAT_EQ(phase.read(), 0.00F);

    // Brake
    a3921.state(State::Brake);
    a3921.run();

    EXPECT_TRUE(is_brake(pwmh, pwml));
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

    EXPECT_TRUE(is_coast(pwmh, pwml));

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

    EXPECT_TRUE(is_brake(pwmh, pwml));
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
