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

}  // namespace
