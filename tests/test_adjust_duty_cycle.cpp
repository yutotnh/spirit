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
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::OFF);
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::OFF);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        current_state      = Motor::State::CCW;
        current_duty_cycle = 0.80F;

        // デューティー比と回転方向の変化は2段階で起きる
        // そのため、2回 adjust_duty_cycle() を呼び出す
        // - 1段階目 ... 回転方向の変化
        // - 2段階目 ... デューティー比の変化
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        current_state      = next_state;
        current_duty_cycle = next_duty_cycle;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        current_state      = next_state;
        current_duty_cycle = next_duty_cycle;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
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
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle - motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall));

        // Brake状態から、Stateとデューティー比が変化することの確認
        current_state      = Motor::State::Brake;
        current_duty_cycle = 0.00F;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        current_state      = next_state;
        current_duty_cycle = next_duty_cycle;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
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
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::OFF);
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::OFF);

        Motor::State current_state;
        float        current_duty_cycle;

        Motor::State next_state;
        float        next_duty_cycle;

        current_state      = Motor::State::CW;
        current_duty_cycle = 1.00F;

        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.10F;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
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
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle,
                        current_duty_cycle + motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise));

        // 最後にデューティー比が目標値に達していることを確認する(上昇)
        current_state      = Motor::State::CW;
        current_duty_cycle = 0.60F;
        std::size_t count  = std::ceil(fabs(motor.get_duty_cycle() - current_duty_cycle) /
                                       motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise));

        // デューティー比が目標値に達するまでループ
        for (std::size_t i = 0; i < count; i++) {
            // ほんとはループ中にデューティー比が目標値に達しているか確認するべきだが、
            // 浮動小数点の誤差があるので、最後の何回かはFAILしてしまう
            // なので、ループの最後でのみ確認する
            adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall),
                              current_state, current_duty_cycle, next_state, next_duty_cycle);
            current_duty_cycle = next_duty_cycle;
        }
        EXPECT_EQ(next_state, motor.get_state());
        EXPECT_FLOAT_EQ(next_duty_cycle, motor.get_duty_cycle());

        // 目標 - 現在 > - 最大下降変化量
        current_state      = Motor::State::CW;
        current_duty_cycle = 0.90F;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
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
            adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall),
                              current_state, current_duty_cycle, next_state, next_duty_cycle);
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

        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle, current_duty_cycle - change_level_fall);

        current_state      = Motor::State::CW;
        current_duty_cycle = 0.50F;
        adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                          motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall), current_state,
                          current_duty_cycle, next_state, next_duty_cycle);
        EXPECT_EQ(next_state, current_state);
        EXPECT_FLOAT_EQ(next_duty_cycle, current_duty_cycle + change_level_rise);
    }
}

/**
 * @brief 正常系のテスト @n
 * 最終的にデューティー比が目標値まで達することの確認 @n
 * 途中経過や、特定の回数できっかり目標値に達成するかは気にしない
 */
TEST(AdjustDutyCycle, Normal)
{
    auto test = [](const Motor::State target_state, const float target_duty_cycle, const float max_rise_delta,
                   const float max_fall_delta, const Motor::State current_state, const float current_duty_cycle,
                   const Motor::State expected_state, const float expected_duty_cycle, const size_t loop_count) {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.state(target_state);
        motor.duty_cycle(target_duty_cycle);

        motor.change_level(Motor::ChangeLevelTarget::Rise, max_rise_delta);
        motor.change_level(Motor::ChangeLevelTarget::Fall, max_fall_delta);

        Motor::State current_state_for_loop      = current_state;
        float        current_duty_cycle_for_loop = current_duty_cycle;
        Motor::State next_state;
        float        next_duty_cycle;

        for (size_t i = 0; i < loop_count; i++) {
            adjust_duty_cycle(motor.get_state(), motor.get_duty_cycle(),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Rise),
                              motor.get_maximum_change_duty_cycle(spirit::Motor::ChangeLevelTarget::Fall),
                              current_state_for_loop, current_duty_cycle_for_loop, next_state, next_duty_cycle);
            current_state_for_loop      = next_state;
            current_duty_cycle_for_loop = next_duty_cycle;
        }

        EXPECT_EQ(next_state, expected_state)
            << "target_state: " << static_cast<int>(target_state) << ", target_duty_cycle: " << target_duty_cycle
            << ", max_rise_delta: " << max_rise_delta << ", max_fall_delta: " << max_fall_delta
            << ", current_state(argument): " << static_cast<int>(current_state)
            << ", current_duty_cycle(argument): " << current_duty_cycle
            << ", current_state(previous): " << static_cast<int>(current_state)
            << ", current_duty_cycle(previous): " << current_duty_cycle
            << ", expected_state: " << static_cast<int>(expected_state)
            << ", expected_duty_cycle: " << expected_duty_cycle << ", loop_count: " << loop_count;
        EXPECT_FLOAT_EQ(next_duty_cycle, expected_duty_cycle)
            << "target_state: " << static_cast<int>(target_state) << ", target_duty_cycle: " << target_duty_cycle
            << ", max_rise_delta: " << max_rise_delta << ", max_fall_delta: " << max_fall_delta
            << ", current_state(argument): " << static_cast<int>(current_state)
            << ", current_duty_cycle(argument): " << current_duty_cycle
            << ", current_state(previous): " << static_cast<int>(current_state)
            << ", current_duty_cycle(previous): " << current_duty_cycle
            << ", expected_state: " << static_cast<int>(expected_state)
            << ", expected_duty_cycle: " << expected_duty_cycle << ", loop_count: " << loop_count;
    };

    float        duty_cycles[] = {0.00F, 0.30F, 1.00F};
    Motor::State states[]      = {Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake};

    auto max_delta = [](Motor::ChangeLevel level) -> float {
        Motor motor;
        motor.control_system(Motor::ControlSystem::PWM);
        motor.change_level(Motor::ChangeLevelTarget::Rise, level);
        return motor.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise);
    };

    float max_deltas[] = {max_delta(Motor::ChangeLevel::OFF),    max_delta(Motor::ChangeLevel::Low),
                          max_delta(Motor::ChangeLevel::Middle), max_delta(Motor::ChangeLevel::High),
                          max_delta(Motor::ChangeLevel::Max),    0.50F};

    // 最終的にデューティー比が目標値まで達することの確認
    // 途中経過や、特定の回数できっかり目標値に達成するかは気にしない
    {
        for (auto target_duty_cycle : duty_cycles) {
            for (auto target_state : states) {
                for (auto max_rise_delta : max_deltas) {
                    for (auto max_fall_delta : max_deltas) {
                        for (auto current_duty_cycle : duty_cycles) {
                            for (auto current_state : states) {
                                // 回転方向がCoastまたはBrakeの場合デューティー比は0になるのでテストをスキップする
                                if ((target_state == Motor::State::Coast) || (target_state == Motor::State::Brake)) {
                                    continue;
                                }

                                // max_rise_delta と max_fall_delta の小さいほうを使用する
                                float delta = std::min(max_rise_delta, max_fall_delta);
                                // 100% -> 0% -> 100% の場合のループ回数を計算する
                                uint32_t loop_count = static_cast<uint32_t>(std::ceil(1.00F / delta)) * 2;
                                // 浮動小数点数の誤差を考慮して、ループの回数を1.1倍にする(1.01倍は適当)
                                loop_count = static_cast<uint32_t>(std::ceil(loop_count * 1.01F));

                                test(target_state, target_duty_cycle, max_rise_delta, max_fall_delta, current_state,
                                     current_duty_cycle, target_state, target_duty_cycle, loop_count);
                            }
                        }
                    }
                }
            }
        }
    }
}

}  // namespace
