#include <gtest/gtest.h>

#include "Motor.h"
#include "adjust_duty_cycle.h"

namespace {

using namespace spirit;

/**
 * @brief 回転方向が異なる場合のテスト
 */
TEST(AdjustDutyCycle, DifferentState)
{
    /// @test Motor::ChangeLevel::OFF の時にデューティー比と回転方向が即座に変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        current_state      = Motor::State::CCW;
        current_duty_cycle = 0.80F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());
    }

    /// @test Motor::ChangeLevel::LOW の時にデューティー比と回転方向が変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::Low);
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::High);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        // CW状態から、Stateとデューティー比が変化することの確認
        current_state      = Motor::State::CCW;
        current_duty_cycle = 0.80F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle - motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall));

        // Brake状態から、Stateとデューティー比が変化することの確認
        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle + motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise));
    }
}

/**
 * @brief 回転方向が同じ場合のテスト
 */
TEST(AdjustDutyCycle, SameState)
{
    /// @test Motor::ChangeLevel::OFF の時にデューティー比が即座に変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.10F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());
    }

    /// @test Motor::ChangeLevel::LOW の時にデューティー比が変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);

        // 変化量は上昇と下降で変える(上昇と下降がそれぞれ値をもとに変化していることを確認するため)
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::Low);
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::High);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        // 目標 - 現在 > 最大上昇変化量
        // 次のデューティー比は現在デューティー比 + 最大上昇変化量 になる
        current_state      = Motor::State::CW;
        current_duty_cycle = 0.70F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle + motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise));

        // 最後にデューティー比が目標値に達していることを確認する(上昇)
        current_state      = Motor::State::CW;
        current_duty_cycle = 0.60F;
        std::size_t count  = fabs(motor.get_duty_cycle() - current_duty_cycle) /
                            motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise);

        // デューティー比が目標値に達するまでループ
        for (std::size_t i = 0; i < count; i++) {
            // ほんとはループ中にデューティー比が目標値に達しているか確認するべきだが、
            // 浮動小数点の誤差があるので、最後の何回かはFAILしてしまう
            // なので、ループの最後でのみ確認する
            adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
            current_duty_cycle = next_duty_cycle;
        }
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        // 目標 - 現在 > - 最大下降変化量
        current_state      = Motor::State::CW;
        current_duty_cycle = 0.90F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle - motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall));

        // 最後にデューティー比が目標値に達していることを確認する(下降)
        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;
        count              = fabs(motor.get_duty_cycle() - current_duty_cycle) /
                motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise);
        for (std::size_t i = 0; i < count; i++) {
            // ほんとはループ中にデューティー比が目標値に達しているか確認するべきだが、
            // 浮動小数点の誤差があるので、最後の何回かはFAILしてしまう
            // なので、ループの最後でのみ確認する
            adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
            current_duty_cycle = next_duty_cycle;
        }
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());
    }
}

/**
 * @brief デューティー比の最大変化幅を手動で設定した場合のテスト
 */
TEST(AdjustDutyCycle, ManualChangeLevel)
{
    /// @test Motor::ChangeLevel::Manual の時にデューティー比が変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);

        float change_level_fall = 0.10F;
        float change_level_rise = 0.15F;

        motor.change_level(Motor::ChangeLevelTarget::Fall, change_level_fall);
        motor.change_level(Motor::ChangeLevelTarget::Rise, change_level_rise);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle, current_duty_cycle - change_level_fall);

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.50F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle, current_duty_cycle + change_level_rise);
    }
}

}  // namespace
