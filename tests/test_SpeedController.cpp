#include <gtest/gtest.h>

#include "Error.h"
#include "SpeedController.h"
#include "tests/stubs.h"

namespace {

using namespace spirit;

TEST(SpeedController, SpeedControllerTest)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase);

    // 計算しやすいようにゲインを設定
    speed_controller.pid_gain(1.00f, 1.00f, 1.00f);
    speed_controller.limit(5.00f, 0.00f);

    /* エンコーダの仮想パルス生成 (1.8°回転) */
    a_phase.interrupt_rise();
    b_phase.interrupt_rise();
    a_phase.interrupt_fall();
    b_phase.interrupt_fall();
    /* ここまで */

    // 角度
    EXPECT_FLOAT_EQ(speed_controller.angle(), 1.80f);
    // PID計算(リミットなし)
    // p = 1.0, i = 1.0(積分は誤差×dt), d = -0.005(微分は測定値 rps に対して行う)
    EXPECT_FLOAT_EQ(speed_controller.calculation(1.005f, 1.00f), 1.995f);
    // RPS
    EXPECT_FLOAT_EQ(speed_controller.rps(), 0.005f);

    speed_controller.limit(1.00f, 0.00f);

    // PID計算(上限リミット)
    EXPECT_FLOAT_EQ(speed_controller.calculation(1.00f, 1.00f), 1.00f);

    speed_controller.reset();

    speed_controller.limit(1.00f, -1.00f);

    /* エンコーダの仮想パルス生成 (-1.8°回転) */
    b_phase.interrupt_rise();
    a_phase.interrupt_rise();
    b_phase.interrupt_fall();
    a_phase.interrupt_fall();
    /* ここまで */

    // 角度(マイナス)
    EXPECT_FLOAT_EQ(speed_controller.angle(), -1.80f);
    // PID計算(リミットなし)

    EXPECT_FLOAT_EQ(speed_controller.calculation(0.00f, 1.00f), -0.015f);

    // RPS(マイナスなし)
    EXPECT_FLOAT_EQ(speed_controller.rps(), 0.005f);

    speed_controller.limit(1.00f, 0.00f);
    // PID計算(下限リミット)
    EXPECT_FLOAT_EQ(speed_controller.calculation(0.00f, 1.00f), 0.00f);

    // Errorが発生していないことを確認
    EXPECT_EQ(Error::get_instance().get_status(), spirit::Error::Status::Normal);
}

/// 積分項が、制御周期の刻み方によらず(誤差×経過時間)になることの確認
TEST(SpeedController, IntegralDoesNotDependOnControlPeriod)
{
    auto integral_output = [](const int cycles, const float dt) {
        StubInterruptIn a_phase;
        StubInterruptIn b_phase;
        SpeedController speed_controller(a_phase, b_phase);
        speed_controller.limit(100.0f, -100.0f);
        speed_controller.pid_gain(0.0f, 2.0f, 0.0f);

        float output = 0.0f;
        for (int i = 0; i < cycles; i++) {
            // 回転しないので誤差は常に目標(1 rps)
            output = speed_controller.calculation(1.0f, dt);
        }
        return output;
    };

    // 1秒間、誤差 1 rps を積分すると、ki(2) × 1 = 2
    EXPECT_NEAR(integral_output(100, 0.01f), 2.0f, 1e-3f);
    EXPECT_NEAR(integral_output(10, 0.1f), 2.0f, 1e-3f);
}

TEST(SpeedController, SpeedControllerErrorTest)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase);

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    Error& error = Error::get_instance();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    speed_controller.limit(0.00f, 1.00f);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    error.reset();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    speed_controller.calculation(-1.00f, 1.00f);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    error.reset();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    speed_controller.calculation(1.00f, 0.00f);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

/// limit() を呼ばなくても、既定の上下限(デューティー比の範囲 0.0〜1.0)で出力されることの確認
TEST(SpeedController, DefaultLimits)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase);
    speed_controller.pid_gain(1.0f, 0.0f, 0.0f);

    // 目標が大きく、停止中なので、出力は上限
    EXPECT_FLOAT_EQ(speed_controller.calculation(5.0f, 0.01f), 1.0f);
    // 目標より速いときは、下限
    speed_controller.reset();
    for (int pulse = 0; pulse < 10; pulse++) {
        a_phase.interrupt_rise();
        b_phase.interrupt_rise();
        a_phase.interrupt_fall();
        b_phase.interrupt_fall();
    }
    EXPECT_FLOAT_EQ(speed_controller.calculation(0.0f, 0.01f), 0.0f);
}

/// エンコーダーの分解能(PPR)を指定でき、角度とrpsに反映されることの確認
TEST(SpeedController, PulsesPerRevolution)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase, 100);

    a_phase.interrupt_rise();
    b_phase.interrupt_rise();
    a_phase.interrupt_fall();
    b_phase.interrupt_fall();

    // 4カウントで 360 / 100 = 3.6度 (既定の 200PPR の2倍)
    EXPECT_FLOAT_EQ(speed_controller.angle(), 3.6f);

    // 0以下の場合は、エラーになり、既定値(200PPR)を使う
    testing::internal::CaptureStderr();
    Error::get_instance().reset();
    StubInterruptIn a2;
    StubInterruptIn b2;
    SpeedController invalid(a2, b2, 0);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Error);
    testing::internal::GetCapturedStderr();
    a2.interrupt_rise();
    b2.interrupt_rise();
    a2.interrupt_fall();
    b2.interrupt_fall();
    EXPECT_FLOAT_EQ(invalid.angle(), 1.8f);
    Error::get_instance().reset();
}

/// SpeedController が割り込みの処理を登録し、破棄されると解除することの確認
TEST(SpeedController, RegistersAndUnregistersHandlers)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;

    {
        SpeedController speed_controller(a_phase, b_phase);
        EXPECT_NE(a_phase.rise_handler(), nullptr);
        EXPECT_EQ(a_phase.rise_handler(), a_phase.fall_handler());  // 立ち上がりと立ち下がりで同じ処理
        EXPECT_NE(b_phase.rise_handler(), nullptr);
        EXPECT_EQ(b_phase.rise_handler(), b_phase.fall_handler());
        EXPECT_NE(a_phase.rise_handler(), b_phase.rise_handler());
    }

    // 破棄された後は、割り込みが発生しても、何も呼び出されない(無効なポインタを使わない)
    EXPECT_EQ(a_phase.rise_handler(), nullptr);
    EXPECT_EQ(a_phase.fall_handler(), nullptr);
    EXPECT_EQ(b_phase.rise_handler(), nullptr);
    EXPECT_EQ(b_phase.fall_handler(), nullptr);
    a_phase.interrupt_rise();
    b_phase.interrupt_fall();
}

/// 正転(A相が先行)と逆転(B相が先行)で、カウントの向きが逆になることの確認
TEST(SpeedController, DirectionFromPhaseOrder)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase);

    // 正転: A↑ B↑ A↓ B↓
    a_phase.interrupt_rise();
    EXPECT_FLOAT_EQ(speed_controller.angle(), 0.45f);
    b_phase.interrupt_rise();
    a_phase.interrupt_fall();
    b_phase.interrupt_fall();
    EXPECT_FLOAT_EQ(speed_controller.angle(), 1.8f);

    // 逆転: B↑ A↑ B↓ A↓
    b_phase.interrupt_rise();
    a_phase.interrupt_rise();
    b_phase.interrupt_fall();
    a_phase.interrupt_fall();
    EXPECT_FLOAT_EQ(speed_controller.angle(), 0.0f);
}

}  // namespace
