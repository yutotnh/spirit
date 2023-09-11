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
    speed_controller.limit(3.00f, 0.00f);

    /* エンコーダの仮想パルス生成 (1.8°回転) */
    a_phase.interrupt_rise();
    b_phase.interrupt_rise();
    a_phase.interrupt_fall();
    b_phase.interrupt_fall();
    /* ここまで */

    // 角度
    EXPECT_FLOAT_EQ(speed_controller.angle(), 1.80f);
    // PID計算(リミットなし)
    EXPECT_FLOAT_EQ(speed_controller.calculation(1.005f, 1.00f), 2.5f);
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
    EXPECT_FLOAT_EQ(speed_controller.calculation(0.00f, 1.00f), -0.0125);
    // RPS(マイナスなし)
    EXPECT_FLOAT_EQ(speed_controller.rps(), 0.005f);

    speed_controller.limit(1.00f, 0.00f);
    // PID計算(下限リミット)
    EXPECT_FLOAT_EQ(speed_controller.calculation(0.00f, 1.00f), 0.00f);
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

    error.reset();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    speed_controller.rps(0.00f);
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

}  // namespace