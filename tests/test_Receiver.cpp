#include <gtest/gtest.h>

#include <atomic>
#include <cmath>
#include <thread>
#include <vector>

#include "BitStream.h"
#include "Error.h"
#include "Receiver.h"
#include "bfloat16.h"

namespace {

using namespace spirit;
using protocol::DecodeStatus;
using protocol::Receiver;

using Bytes = std::vector<uint8_t>;

/// kp = ki = kd = value の設定パケット (bfloat16 で誤差なく表せる 1〜256 の整数を使う)
Bytes pid_frame(const float value)
{
    uint8_t   buffer[8];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b110, 3);
    writer.write_evi(1);
    writer.write_evi(48);
    for (int i = 0; i < 3; i++) {
        writer.write(float32_to_bfloat16(value), 16);
    }
    return Bytes(buffer, buffer + writer.bytes());
}

/// PWM周波数 = hz、 release_time = 2 * hz (ms) の設定パケット
Bytes frequency_frame(const uint32_t hz)
{
    uint8_t   buffer[8];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b110, 3);
    writer.write_evi(2);
    writer.write_evi(16);
    writer.write(hz, 16);
    writer.write_evi(3);
    writer.write_evi(16);
    writer.write(hz * 2, 16);
    return Bytes(buffer, buffer + writer.bytes());
}

Bytes pwm_frame(const uint32_t state, const uint32_t duty_16bit)
{
    uint8_t   buffer[3];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b0, 1);
    writer.write(state, 2);
    writer.write(duty_16bit, 16);
    return Bytes(buffer, buffer + writer.bytes());
}

TEST(Receiver, InitialSnapshotIsInitialMotor)
{
    Motor initial;
    initial.duty_cycle(0.25F);
    initial.state(Motor::State::CW);

    Receiver   receiver(initial);
    const auto snapshot = receiver.snapshot();
    EXPECT_FLOAT_EQ(snapshot.motor.get_duty_cycle(), 0.25F);
    EXPECT_EQ(snapshot.motor.get_state(), Motor::State::CW);
    EXPECT_EQ(snapshot.control_packets, 0U);

    // 何度呼んでも同じ
    EXPECT_EQ(receiver.snapshot().motor.get_state(), Motor::State::CW);
}

TEST(Receiver, ReflectsReceivedPacketsIncrementally)
{
    Receiver receiver;

    // 設定と制御パケットは、重ねて反映される
    EXPECT_EQ(receiver.receive(pid_frame(2.0F).data(), 8).status, DecodeStatus::Applied);
    auto  snapshot = receiver.snapshot();
    float kp, ki, kd;
    snapshot.motor.get_pid_gain_factor(kp, ki, kd);
    EXPECT_EQ(kp, 2.0F);
    EXPECT_EQ(snapshot.control_packets, 0U);  // 設定は制御パケットではない

    const Bytes pwm = pwm_frame(1, 65535);
    EXPECT_TRUE(receiver.receive(pwm.data(), pwm.size()).is_control());
    snapshot = receiver.snapshot();
    snapshot.motor.get_pid_gain_factor(kp, ki, kd);
    EXPECT_EQ(kp, 2.0F);  // 前の設定が残っている
    EXPECT_EQ(snapshot.motor.get_state(), Motor::State::CW);
    EXPECT_EQ(snapshot.motor.get_duty_cycle(), 1.0F);
    EXPECT_EQ(snapshot.control_packets, 1U);
}

TEST(Receiver, ManyUpdatesBetweenSnapshots)
{
    // 読み手が取り込まないまま何度更新されても、最新の状態が見える
    Receiver receiver;
    for (uint32_t i = 1; i <= 10; i++) {
        const Bytes frame = pwm_frame(1, i * 1000);
        receiver.receive(frame.data(), frame.size());
    }

    auto snapshot = receiver.snapshot();
    EXPECT_NEAR(snapshot.motor.get_duty_cycle(), 10000.0F / 65535.0F, 1e-6F);
    EXPECT_EQ(snapshot.control_packets, 10U);

    // 続けて更新しても、読み手の1枚は書き換わらない
    const Bytes frame = pwm_frame(2, 65535);
    receiver.receive(frame.data(), frame.size());
    EXPECT_EQ(snapshot.motor.get_state(), Motor::State::CW);
    EXPECT_EQ(receiver.snapshot().motor.get_state(), Motor::State::CCW);
}

TEST(Receiver, RejectedPacketsDoNotChangeSnapshot)
{
    Receiver    receiver;
    const Bytes pwm = pwm_frame(3, 100);
    receiver.receive(pwm.data(), pwm.size());
    const auto before = receiver.snapshot();

    const uint8_t malformed[] = {0xC0, 0x40};        // R6: 残りに1があるのに種類が0
    const uint8_t invalid[]   = {0xA7, 0xF8, 0x00};  // Speed が +inf
    const uint8_t unknown[]   = {0xF5};
    Error::get_instance().reset();
    EXPECT_EQ(receiver.receive(malformed, sizeof(malformed)).status, DecodeStatus::Malformed);
    EXPECT_EQ(receiver.receive(invalid, sizeof(invalid)).status, DecodeStatus::InvalidValue);
    EXPECT_EQ(receiver.receive(unknown, sizeof(unknown)).status, DecodeStatus::UnknownType);
    EXPECT_EQ(receiver.receive(nullptr, 0).status, DecodeStatus::Malformed);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal);

    EXPECT_EQ(receiver.rejected_packets(), 3U);
    EXPECT_EQ(receiver.unknown_type_packets(), 1U);

    const auto after = receiver.snapshot();
    EXPECT_EQ(after.control_packets, before.control_packets);
    EXPECT_EQ(after.motor.get_state(), before.motor.get_state());
    EXPECT_EQ(after.motor.get_duty_cycle(), before.motor.get_duty_cycle());
}

/// 書き手(割り込みを模したスレッド)が更新し続けている間、読み手が取り出す Motor は、常に整合している
/// (更新途中の、一部のフィールドだけが新しい Motor が見えない)
TEST(Receiver, SnapshotIsConsistentWhileWriting)
{
    constexpr int iterations = 100000;

    Receiver          receiver;
    std::atomic<bool> done{false};

    std::thread writer([&]() {
        for (int i = 0; i < iterations; i++) {
            const Bytes pid  = pid_frame(static_cast<float>(1 + (i % 200)));
            const Bytes freq = frequency_frame(static_cast<uint32_t>(10 + (i % 1000)));
            const Bytes pwm  = pwm_frame(static_cast<uint32_t>(i % 4), static_cast<uint32_t>(i % 65536));
            receiver.receive(pid.data(), pid.size());
            receiver.receive(freq.data(), freq.size());
            receiver.receive(pwm.data(), pwm.size());
        }
        done.store(true);
    });

    uint32_t last_control_packets = 0;
    uint64_t checked              = 0;
    uint64_t torn                 = 0;
    uint64_t went_back            = 0;

    auto check = [&]() {
        const auto snapshot = receiver.snapshot();

        // 3つのゲインは、同じ設定パケットで同時に更新される
        // (まだ設定を受け取っていない状態は、 Motor の既定値のまま)
        float kp, ki, kd;
        snapshot.motor.get_pid_gain_factor(kp, ki, kd);
        const Motor initial;
        float       dkp, dki, dkd;
        initial.get_pid_gain_factor(dkp, dki, dkd);
        const bool is_initial = (kp == dkp) && (ki == dki) && (kd == dkd);
        if (!is_initial && ((kp != ki) || (ki != kd))) {
            torn++;
        }

        // PWM周波数と release_time も、同じ設定パケットで同時に更新される (release_time[ms] = 2 * 周波数)
        const float hz = std::floor(1.0F / snapshot.motor.get_pulse_period() + 0.5F);
        const float ms = std::floor(snapshot.motor.get_release_time() * 1000.0F + 0.5F);
        if (ms != 2.0F * hz) {
            // 初期値 (5000Hz、 500ms) は、まだ設定を受け取っていない状態
            if (!((hz == 5000.0F) && (ms == 500.0F))) {
                torn++;
            }
        }

        // 一度見えた状態より古い状態には戻らない
        if (snapshot.control_packets < last_control_packets) {
            went_back++;
        }
        last_control_packets = snapshot.control_packets;
        checked++;
    };

    while (!done.load()) {
        check();
    }
    writer.join();
    check();

    EXPECT_EQ(torn, 0U);
    EXPECT_EQ(went_back, 0U);
    EXPECT_GT(checked, 0U);
    EXPECT_EQ(receiver.snapshot().control_packets, static_cast<uint32_t>(iterations));
}

}  // namespace
