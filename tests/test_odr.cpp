// static constexpr メンバーを参照渡し(odr-use)しても、リンクできることの確認 (C++14 ではクラス外の定義が必要)

#include <gtest/gtest.h>

#include <algorithm>

#include "CANMessage.h"
#include "Motor.h"
#include "SpeedController.h"

namespace {

using namespace spirit;

/// 引数を参照で受け取る(値のコピーではなく、オブジェクトのアドレスが必要になる)
template <typename T>
const T& identity(const T& value)
{
    return value;
}

TEST(Odr, MotorConstants)
{
    EXPECT_FLOAT_EQ(identity(Motor::Default::kp), 0.30F);
    EXPECT_FLOAT_EQ(identity(Motor::Default::ki), 0.80F);
    EXPECT_FLOAT_EQ(identity(Motor::Default::kd), 0.20F);
    EXPECT_FLOAT_EQ(identity(Motor::Default::pulse_period), 1.0F / 5000.0F);
    EXPECT_FLOAT_EQ(identity(Motor::Default::release_time), 0.5F);
    EXPECT_FLOAT_EQ(identity(Motor::Default::full_scale_time), 0.0F);
    EXPECT_EQ(identity(Motor::Default::control_system), Motor::ControlSystem::PWM);
    EXPECT_EQ(identity(Motor::Default::state), Motor::State::Brake);
    EXPECT_EQ(identity(Motor::Default::rise_change_level), Motor::ChangeLevel::OFF);
    EXPECT_EQ(identity(Motor::Default::fall_change_level), Motor::ChangeLevel::OFF);
    EXPECT_EQ(identity(Motor::Default::decay), Motor::Decay::Slow);
    EXPECT_EQ(identity(Motor::Default::pwm_side), Motor::PwmSide::Low);
    EXPECT_FALSE(identity(Motor::Default::reset));
    EXPECT_FALSE(identity(Motor::Default::sleep));
    EXPECT_LT(identity(Motor::min_pulse_period), identity(Motor::max_pulse_period));

    // std::max / std::min は引数を参照で受け取る
    EXPECT_FLOAT_EQ(std::max(Motor::Default::kp, 0.1F), 0.30F);
    EXPECT_FLOAT_EQ(std::min(Motor::min_pulse_period, 1.0F), Motor::min_pulse_period);
}

TEST(Odr, OtherConstants)
{
    EXPECT_EQ(identity(CANMessage::max_data_length), 8);
    EXPECT_EQ(identity(CANMessage::max_id), 0x7FF);
    EXPECT_EQ(identity(SpeedController::default_pulses_per_revolution), 200);
}

}  // namespace
