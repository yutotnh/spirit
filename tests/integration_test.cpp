#include <gtest/gtest.h>

#include <vector>

#include "spirit.h"

namespace {

/// 実際の CAN のフレーム (8バイトまで)
struct CANFrame {
    uint32_t    id;
    uint8_t     data[8];
    std::size_t len;
};

constexpr uint32_t dip_switch = 0x05;

/// 送信側 (Controller): Motor から CAN のフレームを作る
CANFrame controller_control(const spirit::Motor& motor)
{
    CANFrame frame{};
    EXPECT_TRUE(spirit::can::get_motor_id(2, 1, dip_switch, frame.id));
    EXPECT_TRUE(spirit::protocol::encode_control(motor, frame.data, sizeof(frame.data), frame.len));
    return frame;
}

/// 送信側 (Controller): 全設定を1周分のフレームにする
std::vector<CANFrame> controller_config(const spirit::Motor& motor)
{
    std::vector<CANFrame> frames;
    std::size_t           index = 0;
    do {
        CANFrame    frame{};
        std::size_t next_index = 0;
        EXPECT_TRUE(spirit::can::get_motor_id(2, 1, dip_switch, frame.id));
        EXPECT_TRUE(
            spirit::protocol::encode_config(motor, index, frame.data, sizeof(frame.data), frame.len, next_index));
        frames.push_back(frame);
        index = next_index;
    } while ((index != 0) && (frames.size() < 10));
    return frames;
}

/// 受信側 (Peripheral): 自分宛てのフレームを受け取って、 Motor を更新する
spirit::protocol::DecodeResult peripheral(const CANFrame& frame, spirit::Motor& motor)
{
    uint32_t id = 0;
    EXPECT_TRUE(spirit::can::get_motor_id(2, 1, dip_switch, id));
    EXPECT_EQ(frame.id, id);

    return spirit::protocol::decode(frame.data, frame.len, motor);
}

/// CANMessage (MbedのCANMessageを模したクラス) を経由しても、データが変わらないことの確認
CANFrame through_can_message(const CANFrame& frame)
{
    const spirit::CANMessage message(static_cast<uint16_t>(frame.id), frame.data, static_cast<uint8_t>(frame.len));

    CANFrame result{};
    result.id  = message.get_id();
    result.len = message.get_length();
    message.get_data(result.data, sizeof(result.data));
    return result;
}

/**
 * @brief 結合テスト: Controller の Motor を、制御パケットと設定パケットで送り、 Peripheral の Motor と比較する
 */
TEST(IntegrationTest, PwmControl)
{
    spirit::Error::get_instance().reset();

    for (const auto state : {spirit::Motor::State::Coast, spirit::Motor::State::Brake, spirit::Motor::State::CW,
                             spirit::Motor::State::CCW}) {
        for (const float duty_cycle : {0.0F, 0.2F, 0.5F, 0.75F, 1.0F}) {
            spirit::Motor controller_motor;
            controller_motor.control_system(spirit::Motor::ControlSystem::PWM);
            controller_motor.state(state);
            controller_motor.duty_cycle(duty_cycle);

            spirit::Motor received;
            const auto    result = peripheral(through_can_message(controller_control(controller_motor)), received);

            EXPECT_EQ(result.status, spirit::protocol::DecodeStatus::Applied);
            EXPECT_TRUE(result.is_control());
            EXPECT_EQ(received.get_control_system(), spirit::Motor::ControlSystem::PWM);
            EXPECT_EQ(received.get_state(), state);
            // デューティー比は16bitで表現しているため、量子化の幅の半分の誤差を許容する
            EXPECT_NEAR(received.get_duty_cycle(), duty_cycle, 0.5F / 65535.0F + 1e-7F);
        }
    }

    EXPECT_EQ(spirit::Error::get_instance().get_status(), spirit::Error::Status::Normal);
}

TEST(IntegrationTest, SpeedControl)
{
    spirit::Error::get_instance().reset();

    for (const auto state : {spirit::Motor::State::Coast, spirit::Motor::State::Brake, spirit::Motor::State::CW,
                             spirit::Motor::State::CCW}) {
        for (const float speed : {0.2F, 0.75F, 1.0F, 12.5F}) {
            spirit::Motor controller_motor;
            controller_motor.control_system(spirit::Motor::ControlSystem::Speed);
            controller_motor.state(state);
            controller_motor.speed(speed);

            spirit::Motor received;
            const auto    result = peripheral(through_can_message(controller_control(controller_motor)), received);

            EXPECT_EQ(result.status, spirit::protocol::DecodeStatus::Applied);
            EXPECT_EQ(received.get_control_system(), spirit::Motor::ControlSystem::Speed);
            EXPECT_EQ(received.get_state(), state);
            // bfloat16 の仮数部は7bitなので、その分の誤差を許容する (受信側の値と比較する)
            EXPECT_NEAR(received.get_speed(), speed, speed / 127.0F);
        }
    }

    EXPECT_EQ(spirit::Error::get_instance().get_status(), spirit::Error::Status::Normal);
}

/// 設定を送ると、 kd を含む全ての設定が Peripheral に届く
TEST(IntegrationTest, Config)
{
    spirit::Error::get_instance().reset();

    spirit::Motor controller_motor;
    controller_motor.pid_gain_factor(0.5F, 0.25F, 1.5F);
    controller_motor.pulse_period(1.0F / 30000.0F);
    controller_motor.release_time(1.0F);
    controller_motor.decay(spirit::Motor::Decay::Fast);
    controller_motor.pwm_side(spirit::Motor::PwmSide::High);
    controller_motor.change_level(spirit::Motor::ChangeLevelTarget::Rise, spirit::Motor::ChangeLevel::Middle);
    controller_motor.full_scale_time(spirit::Motor::ChangeLevelTarget::Fall, 2.0F);

    spirit::Motor received;
    for (const auto& frame : controller_config(controller_motor)) {
        EXPECT_EQ(peripheral(through_can_message(frame), received).status, spirit::protocol::DecodeStatus::Applied);
    }

    float kp, ki, kd;
    received.get_pid_gain_factor(kp, ki, kd);
    EXPECT_FLOAT_EQ(kp, 0.5F);
    EXPECT_FLOAT_EQ(ki, 0.25F);
    EXPECT_FLOAT_EQ(kd, 1.5F);
    EXPECT_FLOAT_EQ(received.get_pulse_period(), 1.0F / 30000.0F);
    EXPECT_FLOAT_EQ(received.get_release_time(), 1.0F);
    EXPECT_EQ(received.get_decay(), spirit::Motor::Decay::Fast);
    EXPECT_EQ(received.get_pwm_side(), spirit::Motor::PwmSide::High);
    EXPECT_EQ(received.get_change_level(spirit::Motor::ChangeLevelTarget::Rise), spirit::Motor::ChangeLevel::Middle);
    EXPECT_EQ(received.get_change_level(spirit::Motor::ChangeLevelTarget::Fall), spirit::Motor::ChangeLevel::Manual);
    EXPECT_FLOAT_EQ(received.get_full_scale_time(spirit::Motor::ChangeLevelTarget::Fall), 2.0F);

    EXPECT_EQ(spirit::Error::get_instance().get_status(), spirit::Error::Status::Normal);
}

/// 設定のフレームを1つ落としても、次の周期の再送で正しい状態に戻る
TEST(IntegrationTest, LostConfigFrameIsRecoveredByResend)
{
    spirit::Motor controller_motor;
    controller_motor.pid_gain_factor(1.0F, 2.0F, 0.5F);
    controller_motor.release_time(1.0F);
    controller_motor.decay(spirit::Motor::Decay::Fast);
    controller_motor.sleep(true);

    const std::vector<CANFrame> frames = controller_config(controller_motor);
    ASSERT_GE(frames.size(), 3U);

    // 1周目: 2番目のフレーム(pwm_frequency と release_time)を落とす
    spirit::Motor received;
    for (std::size_t i = 0; i < frames.size(); i++) {
        if (i != 1) {
            peripheral(frames[i], received);
        }
    }
    EXPECT_NE(received.get_release_time(), controller_motor.get_release_time());  // 落としたので、まだ既定値

    // 2周目: 全てそろう
    for (const auto& frame : frames) {
        peripheral(frame, received);
    }
    EXPECT_FLOAT_EQ(received.get_release_time(), controller_motor.get_release_time());
    EXPECT_EQ(received.get_decay(), spirit::Motor::Decay::Fast);
    EXPECT_TRUE(received.get_sleep());
}

/// 制御パケットと設定パケットが混ざっても、それぞれ正しく適用される
TEST(IntegrationTest, ControlAndConfigInterleaved)
{
    spirit::Motor controller_motor;
    controller_motor.control_system(spirit::Motor::ControlSystem::PWM);
    controller_motor.state(spirit::Motor::State::CW);
    controller_motor.duty_cycle(1.0F);
    controller_motor.decay(spirit::Motor::Decay::Fast);

    spirit::Motor received;
    const auto    config = controller_config(controller_motor);
    for (const auto& frame : config) {
        peripheral(frame, received);
        const auto result = peripheral(controller_control(controller_motor), received);
        EXPECT_TRUE(result.is_control());
    }

    EXPECT_EQ(received.get_state(), spirit::Motor::State::CW);
    EXPECT_FLOAT_EQ(received.get_duty_cycle(), 1.0F);
    EXPECT_EQ(received.get_decay(), spirit::Motor::Decay::Fast);
}

}  // namespace
