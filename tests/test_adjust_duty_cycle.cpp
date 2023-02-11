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

        Motor::State nest_state;
        float        nest_duty_cycle;

        current_state      = Motor::State::CCW;
        current_duty_cycle = 0.80F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, motor.get_state());
        EXPECT_FLOAT_EQ(nest_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, motor.get_state());
        EXPECT_FLOAT_EQ(nest_duty_cycle, motor.get_duty_cycle());
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

        Motor::State nest_state;
        float        nest_duty_cycle;

        current_state      = Motor::State::CCW;
        current_duty_cycle = 0.80F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, current_state);
        EXPECT_FLOAT_EQ(nest_duty_cycle,
                        current_duty_cycle - motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall));

        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, motor.get_state());
        EXPECT_FLOAT_EQ(nest_duty_cycle,
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

        Motor::State nest_state;
        float        nest_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, motor.get_state());
        EXPECT_FLOAT_EQ(nest_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.10F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, motor.get_state());
        EXPECT_FLOAT_EQ(nest_duty_cycle, motor.get_duty_cycle());
    }

    /// @test Motor::ChangeLevel::LOW の時にデューティー比が変更されることの確認
    {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(Motor::State::CW);
        motor.duty_cycle(0.80F);
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::Low);
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::High);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State nest_state;
        float        nest_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.90F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, current_state);
        EXPECT_FLOAT_EQ(nest_duty_cycle,
                        current_duty_cycle - motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall));

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.70F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, current_state);
        EXPECT_FLOAT_EQ(nest_duty_cycle,
                        current_duty_cycle + motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise));
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

        Motor::State nest_state;
        float        nest_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;

        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, current_state);
        EXPECT_FLOAT_EQ(nest_duty_cycle, current_duty_cycle - change_level_fall);

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.50F;
        adjust_duty_cycle(motor, current_state, current_duty_cycle, nest_state, nest_duty_cycle);
        EXPECT_EQ(nest_state, current_state);
        EXPECT_FLOAT_EQ(nest_duty_cycle, current_duty_cycle + change_level_rise);
    }
}

}  // namespace
