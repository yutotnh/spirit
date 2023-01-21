#include <gtest/gtest.h>

#include "Error.h"
#include "Motor.h"

namespace {

using namespace spirit;

/**
 * @brief 初期値のテスト
 */
TEST(Motor, InitValueTest)
{
    Motor motor;

    auto control_system = Motor::Default::control_system;
    EXPECT_EQ(motor.get_control_system(), control_system);

    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 0.00F);

    EXPECT_FLOAT_EQ(motor.get_speed(), 0.00F);

    float Kp, Ki, Kd;
    motor.get_pid_gain_factor(Kp, Ki, Kd);
    EXPECT_FLOAT_EQ(Kp, Motor::Default::Kp);
    EXPECT_FLOAT_EQ(Ki, Motor::Default::Ki);
    EXPECT_FLOAT_EQ(Kd, Motor::Default::Kd);

    // enum class や bool で定義した型を使うと、以下のようなエラーになるので、一旦変数に保存させる
    //      undefined reference to `spirit::Motor::Default::rise_change_level'

    auto state = Motor::Default::state;
    EXPECT_EQ(motor.get_state(), state);

    auto rise = Motor::Default::rise_change_level;
    EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Rise), rise);

    auto fall = Motor::Default::fall_change_level;
    EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Fall), fall);

    EXPECT_FLOAT_EQ(motor.get_pulse_period(), Motor::Default::pulse_period);

    EXPECT_FLOAT_EQ(motor.get_release_time(), Motor::Default::release_time);

    auto decay = Motor::Default::decay;
    EXPECT_EQ(motor.get_decay(), decay);

    auto pwm_side = Motor::Default::pwm_side;
    EXPECT_EQ(motor.get_pwm_side(), pwm_side);

    auto reset = Motor::Default::reset;
    EXPECT_EQ(motor.get_reset(), reset);

    auto sleep = Motor::Default::sleep;
    EXPECT_EQ(motor.get_sleep(), sleep);
}

/**
 * @brief Motor::control_system(), Motor::get_control_system() のテスト
 */
TEST(Motor, ControlSystemTest)
{
    Motor motor;

    motor.control_system(Motor::ControlSystem::PWM);
    EXPECT_EQ(motor.get_control_system(), Motor::ControlSystem::PWM);

    motor.control_system(Motor::ControlSystem::Speed);
    EXPECT_EQ(motor.get_control_system(), Motor::ControlSystem::Speed);

    // 初期値が Motor::ControlSystem::PWM なので、Motor::ControlSystem::PWM 以外を指定しても Motor::ControlSystem::PWM になる
    motor.control_system(Motor::ControlSystem::PWM);
    EXPECT_EQ(motor.get_control_system(), Motor::ControlSystem::PWM);

    // 異常系のテスト
    Error &error = Error::get_instance();
    EXPECT_EQ(error.get_status(), Error::Status::Normal);
    motor.control_system(static_cast<Motor::ControlSystem>(3));
    EXPECT_EQ(error.get_status(), Error::Status::Error);
}

/**
 * @brief Motor::duty_cycle(), Motor::get_duty_cycle() のテスト
 */
TEST(Motor, DutyCycleTest)
{
    Motor motor;

    motor.duty_cycle(0.00F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 0.00F);

    motor.duty_cycle(0.50F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 0.50F);

    motor.duty_cycle(1.00F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 1.00F);

    // 境界値テスト
    // 0.00F未満
    motor.duty_cycle(-0.01F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 0.00F);
    motor.duty_cycle(-0.50F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 0.00F);

    // 1.00Fより大きい
    motor.duty_cycle(1.01F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 1.00F);
    motor.duty_cycle(2.00F);
    EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 1.00F);
}

/**
 * @brief Motor::speed(), Motor::get_speed() のテスト
 */
TEST(Motor, speedTest)
{
    Motor motor;

    motor.speed(0.00F);
    EXPECT_FLOAT_EQ(motor.get_speed(), 0.00F);

    motor.speed(0.50F);
    EXPECT_FLOAT_EQ(motor.get_speed(), 0.50F);

    motor.speed(1.00F);
    EXPECT_FLOAT_EQ(motor.get_speed(), 1.00F);

    motor.speed(-0.50F);
    EXPECT_FLOAT_EQ(motor.get_speed(), -0.50F);
}

/**
 * @brief Motor::pid_gein_factor(), Motor::get_pid_gain_factor() のテスト
 */
TEST(Motor, PidGainFactorTest)
{
    auto test = [](float Kp, float Ki, float Kd) {
        Motor motor;
        motor.pid_gain_factor(Kp, Ki, Kd);
        motor.get_pid_gain_factor(Kp, Ki, Kd);
        EXPECT_FLOAT_EQ(Kp, Kp);
        EXPECT_FLOAT_EQ(Ki, Ki);
        EXPECT_FLOAT_EQ(Ki, Ki);
    };

    // とりあえず、Kp, Ki, Kdが異なる適当な値を入れてテスト
    test(0.00F, 0.01F, 0.02F);
    test(0.50F, 0.51F, 0.52F);
    test(1000.00F, 100.00F, 10.00F);
}

/**
 * @brief Motor::state(), Motor::get_state()のテスト
 */
TEST(Motor, StateTest)
{
    Motor motor;

    motor.state(Motor::State::Coast);
    EXPECT_EQ(motor.get_state(), Motor::State::Coast);

    motor.state(Motor::State::CW);
    EXPECT_EQ(motor.get_state(), Motor::State::CW);

    motor.state(Motor::State::CCW);
    EXPECT_EQ(motor.get_state(), Motor::State::CCW);

    motor.state(Motor::State::Brake);
    EXPECT_EQ(motor.get_state(), Motor::State::Brake);

    // 初期値が Motor::State::Coast なので、Motor::State::Coast 以外を指定しても Motor::State::Coast になる
    motor.state(Motor::State::Coast);
}

/**
 * @brief Rise, Fall にそれぞれ値を入れてテスト
 * @details Motor::ChangeLevel が5種類あるので、Rise 5パターン * Fall 5パターン = 25 パターン @n
 * また、Fallに設定した値が Rise に設定されているかも確認する(もちろん逆も)
 */
TEST(Motor, ChangeLevelTest)
{
    Motor              motor;
    Motor::ChangeLevel levels[] = {Motor::ChangeLevel::OFF,  Motor::ChangeLevel::Low, Motor::ChangeLevel::Middle,
                                   Motor::ChangeLevel::High, Motor::ChangeLevel::Max, Motor::ChangeLevel::OFF};

    // Fallに設定しても、Riseには反映されないことを確認
    for (const auto &rise : levels) {
        for (const auto &fall : levels) {
            motor.change_level(Motor::ChangeLevelTarget::Rise, rise);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Rise), rise);

            motor.change_level(Motor::ChangeLevelTarget::Fall, fall);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Fall), fall);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Rise), rise);
        }
    }

    // Riseに設定しても、Fallには反映されないことを確認
    for (const auto &rise : levels) {
        for (const auto &fall : levels) {
            motor.change_level(Motor::ChangeLevelTarget::Fall, fall);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Fall), fall);

            motor.change_level(Motor::ChangeLevelTarget::Rise, rise);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Rise), rise);
            EXPECT_EQ(motor.get_change_level(Motor::ChangeLevelTarget::Fall), fall);
        }
    }
}

/**
 * @brief Motor::pulse_width(), Motor::get_pulse_width() のテスト
 */
TEST(Motor, PulsePeriodTest)
{
    Motor motor;

    constexpr float _1kHz = 1.0F / 1.0e3F;
    motor.pulse_period(_1kHz);
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), _1kHz);

    constexpr float _10kHz = 1.0F / 10.0e3F;
    motor.pulse_period(_10kHz);
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), _10kHz);

    // 境界値テスト
    // Motor::min_pulse_period 未満
    motor.pulse_period(Motor::min_pulse_period - (Motor::min_pulse_period * 0.1F));
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), Motor::min_pulse_period);
    motor.pulse_period(Motor::min_pulse_period - (Motor::min_pulse_period * 10.0F));
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), Motor::min_pulse_period);

    // max_pulse_period より大きい
    motor.pulse_period(Motor::max_pulse_period + (Motor::max_pulse_period * 0.1F));
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), Motor::max_pulse_period);
    motor.pulse_period(Motor::max_pulse_period + (Motor::max_pulse_period * 10.0F));
    EXPECT_FLOAT_EQ(motor.get_pulse_period(), Motor::max_pulse_period);
}

/**
 * @brief Motor::release_time(), Motor::get_release_time() のテスト
 */
TEST(Motor, ReleaseTimeTest)
{
    Motor motor;
    motor.release_time(1.00F);
    EXPECT_EQ(motor.get_release_time(), 1.00F);
}

/**
 * @brief Motor::decay(), Motor::get_decay() のテスト
 */
TEST(Motor, DecayTest)
{
    Motor motor;
    motor.decay(Motor::Decay::Slow);
    EXPECT_EQ(motor.get_decay(), Motor::Decay::Slow);

    motor.decay(Motor::Decay::Fast);
    EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
}

/**
 * @brief Motor::pwm_side(), Motor::get_pwm_side() のテスト
 */
TEST(Motor, PwmSideTest)
{
    Motor motor;
    motor.pwm_side(Motor::PwmSide::Low);
    EXPECT_EQ(motor.get_pwm_side(), Motor::PwmSide::Low);

    motor.pwm_side(Motor::PwmSide::High);
    EXPECT_EQ(motor.get_pwm_side(), Motor::PwmSide::High);
}

/**
 * @brief Motor::reset(), Motor::get_reset() のテスト
 */
TEST(Motor, ResetTest)
{
    Motor motor;
    motor.reset(true);
    EXPECT_EQ(motor.get_reset(), true);

    motor.reset(false);
    EXPECT_EQ(motor.get_reset(), false);
}

/**
 * @brief Motor::sleep(), Motor::get_sleep() のテスト
 */
TEST(Motor, SleepTest)
{
    Motor motor;
    motor.sleep(true);
    EXPECT_EQ(motor.get_sleep(), true);

    motor.sleep(false);
    EXPECT_EQ(motor.get_sleep(), false);
}

}  // namespace
