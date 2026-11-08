#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <random>
#include <vector>

#include "BitStream.h"
#include "Error.h"
#include "Protocol.h"

namespace {

using namespace spirit;
using protocol::DecodeStatus;
using protocol::MessageType;

using Bytes = std::vector<uint8_t>;

/// Motor の全ての設定が一致していることを確認する
void expect_same_motor(const Motor& expected, const Motor& actual, const std::string& context = "")
{
    EXPECT_EQ(expected.get_control_system(), actual.get_control_system()) << context;
    EXPECT_EQ(expected.get_state(), actual.get_state()) << context;
    EXPECT_EQ(expected.get_duty_cycle(), actual.get_duty_cycle()) << context;
    EXPECT_EQ(expected.get_speed(), actual.get_speed()) << context;

    float ekp, eki, ekd, akp, aki, akd;
    expected.get_pid_gain_factor(ekp, eki, ekd);
    actual.get_pid_gain_factor(akp, aki, akd);
    EXPECT_EQ(ekp, akp) << context;
    EXPECT_EQ(eki, aki) << context;
    EXPECT_EQ(ekd, akd) << context;

    for (const auto target : {Motor::ChangeLevelTarget::Rise, Motor::ChangeLevelTarget::Fall}) {
        EXPECT_EQ(expected.get_change_level(target), actual.get_change_level(target)) << context;
        EXPECT_EQ(expected.get_full_scale_time(target), actual.get_full_scale_time(target)) << context;
    }

    EXPECT_EQ(expected.get_pulse_period(), actual.get_pulse_period()) << context;
    EXPECT_EQ(expected.get_release_time(), actual.get_release_time()) << context;
    EXPECT_EQ(expected.get_decay(), actual.get_decay()) << context;
    EXPECT_EQ(expected.get_pwm_side(), actual.get_pwm_side()) << context;
    EXPECT_EQ(expected.get_reset(), actual.get_reset()) << context;
    EXPECT_EQ(expected.get_sleep(), actual.get_sleep()) << context;
}

/// Motor を使い回さずに、毎回 decode する (decode の前後で Error が通知されないことも確認する)
protocol::DecodeResult decode(const Bytes& data, Motor& motor)
{
    Error::get_instance().reset();
    const protocol::DecodeResult result = protocol::decode(data.data(), data.size(), motor);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal) << "decode() は Error を通知しない";
    return result;
}

// ---------------------------------------------------------------------------
// ゴールデンベクタ (PROTOCOL.md 12 章)
// ---------------------------------------------------------------------------

TEST(Protocol, GoldenVectorEncodeControl)
{
    uint8_t     buffer[8];
    std::size_t bytes = 0;

    /// @test 1. PWM、CW、50%
    Motor pwm;
    pwm.duty_cycle(0.5F);
    pwm.state(Motor::State::CW);
    ASSERT_TRUE(protocol::encode_control(pwm, buffer, sizeof(buffer), bytes));
    EXPECT_EQ(Bytes(buffer, buffer + bytes), (Bytes{0x30, 0x00, 0x00}));

    /// @test 7. PWM、Brake、0%
    pwm.duty_cycle(0.0F);
    pwm.state(Motor::State::Brake);
    ASSERT_TRUE(protocol::encode_control(pwm, buffer, sizeof(buffer), bytes));
    EXPECT_EQ(Bytes(buffer, buffer + bytes), (Bytes{0x60, 0x00, 0x00}));

    /// @test 8. PWM、CCW、100%
    pwm.duty_cycle(1.0F);
    pwm.state(Motor::State::CCW);
    ASSERT_TRUE(protocol::encode_control(pwm, buffer, sizeof(buffer), bytes));
    EXPECT_EQ(Bytes(buffer, buffer + bytes), (Bytes{0x5F, 0xFF, 0xE0}));

    /// @test 2. Speed、CCW、12.5 rps
    Motor speed;
    speed.control_system(Motor::ControlSystem::Speed);
    speed.speed(12.5F);
    speed.state(Motor::State::CCW);
    ASSERT_TRUE(protocol::encode_control(speed, buffer, sizeof(buffer), bytes));
    EXPECT_EQ(Bytes(buffer, buffer + bytes), (Bytes{0xA4, 0x14, 0x80}));
}

TEST(Protocol, GoldenVectorDecode)
{
    {
        /// @test 1. PWM、CW、50%
        Motor motor;
        auto  result = decode({0x30, 0x00, 0x00}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.type, MessageType::Pwm);
        EXPECT_TRUE(result.is_control());
        EXPECT_EQ(motor.get_control_system(), Motor::ControlSystem::PWM);
        EXPECT_EQ(motor.get_state(), Motor::State::CW);
        EXPECT_FLOAT_EQ(motor.get_duty_cycle(), 32768.0F / 65535.0F);
    }
    {
        /// @test 2. Speed、CCW、12.5 rps
        Motor motor;
        auto  result = decode({0xA4, 0x14, 0x80}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.type, MessageType::Speed);
        EXPECT_TRUE(result.is_control());
        EXPECT_EQ(motor.get_control_system(), Motor::ControlSystem::Speed);
        EXPECT_EQ(motor.get_state(), Motor::State::CCW);
        EXPECT_FLOAT_EQ(motor.get_speed(), 12.5F);
    }
    {
        /// @test 3. Config、decay=Fast
        Motor motor;
        auto  result = decode({0xC8, 0x50}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.type, MessageType::Config);
        EXPECT_FALSE(result.is_control());
        EXPECT_EQ(result.applied_items, 1);
        EXPECT_EQ(result.skipped_items, 0);
        EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
    }
    {
        /// @test 4a. Config、pwm_frequency=20000 Hz、release_time=500 ms
        Motor motor;
        motor.release_time(1.0F);
        auto result = decode({0xC5, 0x40, 0x9C, 0x40, 0x74, 0x00, 0x3E, 0x80}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.applied_items, 2);
        EXPECT_FLOAT_EQ(motor.get_pulse_period(), 1.0F / 20000.0F);
        EXPECT_FLOAT_EQ(motor.get_release_time(), 0.5F);
    }
    {
        /// @test 4b. Config、decay=Fast、pwm_side=High、sleep=true
        Motor motor;
        auto  result = decode({0xC8, 0x52, 0x8D, 0xC6}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.applied_items, 3);
        EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
        EXPECT_EQ(motor.get_pwm_side(), Motor::PwmSide::High);
        EXPECT_TRUE(motor.get_sleep());
        EXPECT_FALSE(motor.get_reset());
    }
    {
        /// @test 5. Config、未知の種類 50 (長さ 5) を読み飛ばし、 decay=Fast を適用する
        Motor motor;
        auto  result = decode({0xDC, 0x4B, 0x54, 0x28}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.applied_items, 1);
        EXPECT_EQ(result.skipped_items, 1);
        EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
    }
    {
        /// @test 6. Config、pid_gain (kp=1.5、ki=0.25、kd=0.125)
        Motor motor;
        auto  result = decode({0xC3, 0xC0, 0x7F, 0x80, 0x7D, 0x00, 0x7C, 0x00}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        float kp, ki, kd;
        motor.get_pid_gain_factor(kp, ki, kd);
        EXPECT_FLOAT_EQ(kp, 1.5F);
        EXPECT_FLOAT_EQ(ki, 0.25F);
        EXPECT_FLOAT_EQ(kd, 0.125F);
    }
    {
        /// @test 7, 8. PWM、Brake 0% と CCW 100%
        Motor motor;
        EXPECT_EQ(decode({0x60, 0x00, 0x00}, motor).status, DecodeStatus::Applied);
        EXPECT_EQ(motor.get_state(), Motor::State::Brake);
        EXPECT_EQ(motor.get_duty_cycle(), 0.0F);
        EXPECT_EQ(decode({0x5F, 0xFF, 0xE0}, motor).status, DecodeStatus::Applied);
        EXPECT_EQ(motor.get_state(), Motor::State::CCW);
        EXPECT_EQ(motor.get_duty_cycle(), 1.0F);
    }
    {
        /// @test 9. 拡張型 9
        Motor       motor;
        const Motor before = motor;
        auto        result = decode({0xF5}, motor);
        EXPECT_EQ(result.status, DecodeStatus::UnknownType);
        EXPECT_EQ(result.type, MessageType::Unknown);
        expect_same_motor(before, motor);
    }
    {
        /// @test 10. PWM、CW、50% に項目 decay=Fast が相乗りしている (文法上は有効)
        Motor motor;
        auto  result = decode({0x30, 0x00, 0x08, 0x50}, motor);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.type, MessageType::Pwm);
        EXPECT_EQ(result.applied_items, 1);
        EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
        EXPECT_EQ(motor.get_state(), Motor::State::CW);
    }
}

TEST(Protocol, GoldenVectorRejected)
{
    struct Case {
        Bytes        data;
        DecodeStatus status;
        const char*  reason;
    };

    const Case cases[] = {
        {{}, DecodeStatus::Malformed, "R1: 長さ0"},
        {{0x30, 0x00}, DecodeStatus::Malformed, "R3: PWM の固定フィールドが16bitしかない"},
        {{0xA4, 0x14}, DecodeStatus::Malformed, "R3: Speed の固定フィールドが16bitしかない"},
        {{0xC0, 0x40}, DecodeStatus::Malformed, "R6: 残りに1があるのに種類が0"},
        {{0xC2, 0x60, 0x00}, DecodeStatus::Malformed, "R8: pid_gain の長さが3"},
        {{0xC8, 0x48}, DecodeStatus::InvalidValue, "R9: decay=1 (Mixed)"},
        {{0xC8, 0x50, 0xCC}, DecodeStatus::Malformed, "R5/R7: 詰め物が0でない"},
        {{0xC8, 0x51},
         DecodeStatus::Malformed,
         "R5: 末尾の詰め物(3bit)に1が含まれていて、項目として読むと途中で切れる"},
        {{0x30, 0x00, 0x01}, DecodeStatus::Malformed, "R5: PWM の後ろの詰め物(5bit)に1が含まれている"},
        {{0xE0}, DecodeStatus::UnknownType, "R2: 型 1110"},
        {{0xA7, 0xF8, 0x00}, DecodeStatus::InvalidValue, "R4: Speed が +inf"},
    };

    for (const auto& c : cases) {
        Motor motor;
        motor.duty_cycle(0.3F);
        motor.release_time(2.0F);
        const Motor before = motor;

        const auto result = decode(c.data, motor);
        EXPECT_EQ(result.status, c.status) << c.reason;
        EXPECT_FALSE(result.is_control()) << c.reason;
        expect_same_motor(before, motor, c.reason);
    }
}

TEST(Protocol, NullBuffer)
{
    Motor       motor;
    const Motor before = motor;
    Error::get_instance().reset();
    EXPECT_EQ(protocol::decode(nullptr, 0, motor).status, DecodeStatus::Malformed);
    EXPECT_EQ(protocol::decode(nullptr, 8, motor).status, DecodeStatus::Malformed);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal);
    expect_same_motor(before, motor);
}

// ---------------------------------------------------------------------------
// 制御パケットの往復
// ---------------------------------------------------------------------------

TEST(Protocol, ControlRoundTripPwm)
{
    for (const auto state : {Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake}) {
        for (int i = 0; i <= 200; i++) {
            const float duty_cycle = static_cast<float>(i) / 200.0F;

            Motor tx;
            tx.duty_cycle(duty_cycle);
            tx.state(state);

            uint8_t     buffer[8];
            std::size_t bytes = 0;
            ASSERT_TRUE(protocol::encode_control(tx, buffer, sizeof(buffer), bytes));
            EXPECT_EQ(bytes, 3U);

            Motor      rx;
            const auto result = protocol::decode(buffer, bytes, rx);
            ASSERT_EQ(result.status, DecodeStatus::Applied);
            EXPECT_EQ(rx.get_control_system(), Motor::ControlSystem::PWM);
            EXPECT_EQ(rx.get_state(), state);
            // 四捨五入なので、誤差は 16bit の量子化幅の半分以内
            EXPECT_NEAR(rx.get_duty_cycle(), duty_cycle, 0.5F / 65535.0F + 1e-7F);
        }
    }
}

TEST(Protocol, ControlRoundTripSpeed)
{
    for (const auto state : {Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake}) {
        for (const float speed : {0.0F, 0.01F, 0.75F, 1.0F, 12.5F, 100.0F, 3.0e38F}) {
            Motor tx;
            tx.control_system(Motor::ControlSystem::Speed);
            tx.speed(speed);
            tx.state(state);

            uint8_t     buffer[8];
            std::size_t bytes = 0;
            ASSERT_TRUE(protocol::encode_control(tx, buffer, sizeof(buffer), bytes));
            EXPECT_EQ(bytes, 3U);

            Motor      rx;
            const auto result = protocol::decode(buffer, bytes, rx);
            ASSERT_EQ(result.status, DecodeStatus::Applied);
            EXPECT_EQ(rx.get_control_system(), Motor::ControlSystem::Speed);
            EXPECT_EQ(rx.get_state(), state);
            // bfloat16 は切り捨てなので、相対誤差は 1/128 未満
            EXPECT_NEAR(rx.get_speed(), speed, speed / 128.0F);
            EXPECT_LE(rx.get_speed(), speed);
        }
    }
}

/// 使い回したバッファ(全て1)に encode しても、前の内容が残らない (BUGS.md #0 の再発防止)
TEST(Protocol, ControlDoesNotDependOnPreviousBuffer)
{
    for (const auto system : {Motor::ControlSystem::PWM, Motor::ControlSystem::Speed}) {
        for (const auto state : {Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake}) {
            Motor motor;
            motor.control_system(system);
            motor.duty_cycle(0.25F);
            motor.speed(2.0F);
            motor.state(state);

            uint8_t     clean[8] = {};
            uint8_t     dirty[8];
            std::size_t clean_bytes = 0, dirty_bytes = 0;
            std::memset(dirty, 0xFF, sizeof(dirty));

            ASSERT_TRUE(protocol::encode_control(motor, clean, sizeof(clean), clean_bytes));
            ASSERT_TRUE(protocol::encode_control(motor, dirty, sizeof(dirty), dirty_bytes));
            EXPECT_EQ(clean_bytes, dirty_bytes);
            EXPECT_EQ(std::memcmp(clean, dirty, sizeof(clean)), 0);
        }
    }
}

TEST(Protocol, ControlSpeedRejectsInvalidValues)
{
    // 負の速度、無限大、 NaN (bfloat16: 符号ビット、 0x7F80 = +inf、 0xFF80 = -inf、 0x7FC0 = NaN)
    const uint16_t invalid[] = {0xC000 /* -2.0 */, 0x7F80, 0xFF80, 0x7FC0, 0xFFC0};
    for (const uint16_t raw : invalid) {
        uint8_t   buffer[3];
        BitWriter writer(buffer, sizeof(buffer));
        writer.write(0b10, 2);
        writer.write(1, 2);
        writer.write(raw, 16);

        Motor motor;
        motor.speed(5.0F);
        const Motor before = motor;
        EXPECT_EQ(protocol::decode(buffer, sizeof(buffer), motor).status, DecodeStatus::InvalidValue) << raw;
        expect_same_motor(before, motor);
    }

    // -0 は 0 として受け付ける
    uint8_t   buffer[3];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b10, 2);
    writer.write(1, 2);
    writer.write(0x8000, 16);
    Motor motor;
    EXPECT_EQ(protocol::decode(buffer, sizeof(buffer), motor).status, DecodeStatus::Applied);
    EXPECT_EQ(motor.get_speed(), 0.0F);
}

TEST(Protocol, EncodeControlFailures)
{
    Motor       motor;
    uint8_t     buffer[8];
    std::size_t bytes = 99;

    testing::internal::CaptureStderr();
    EXPECT_FALSE(protocol::encode_control(motor, buffer, 2, bytes));
    EXPECT_FALSE(protocol::encode_control(motor, nullptr, 8, bytes));
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(bytes, 99U);
    EXPECT_TRUE(protocol::encode_control(motor, buffer, 3, bytes));
}

// ---------------------------------------------------------------------------
// 設定パケット
// ---------------------------------------------------------------------------

/// 全設定を1周分送って、受信側の Motor に適用する。使ったフレーム数を返す
std::size_t send_all_config(const Motor& tx, Motor& rx, const std::size_t max_bytes)
{
    std::size_t frames = 0;
    std::size_t index  = 0;
    do {
        uint8_t     buffer[64];
        std::size_t bytes = 0, next_index = 0;
        EXPECT_TRUE(protocol::encode_config(tx, index, buffer, max_bytes, bytes, next_index));
        EXPECT_LE(bytes, max_bytes);

        const auto result = protocol::decode(buffer, bytes, rx);
        EXPECT_EQ(result.status, DecodeStatus::Applied);
        EXPECT_EQ(result.type, MessageType::Config);
        EXPECT_FALSE(result.is_control());

        index = next_index;
        frames++;
    } while ((index != 0) && (frames < 20));

    return frames;
}

/// 設定が量子化(bfloat16、ミリ秒、整数Hz)で変わらない値を使った Motor
Motor make_configured_motor(const bool rise_manual, const bool fall_manual)
{
    Motor motor;
    motor.pid_gain_factor(1.5F, 0.25F, 0.125F);
    motor.pulse_period(1.0F / 20000.0F);
    motor.release_time(0.25F);
    motor.decay(Motor::Decay::Fast);
    motor.pwm_side(Motor::PwmSide::High);
    motor.reset(true);
    motor.sleep(true);

    if (rise_manual) {
        motor.full_scale_time(Motor::ChangeLevelTarget::Rise, 1.5F);
    } else {
        motor.change_level(Motor::ChangeLevelTarget::Rise, Motor::ChangeLevel::High);
    }

    if (fall_manual) {
        motor.full_scale_time(Motor::ChangeLevelTarget::Fall, 0.75F);
    } else {
        motor.change_level(Motor::ChangeLevelTarget::Fall, Motor::ChangeLevel::Low);
    }

    return motor;
}

TEST(Protocol, ConfigRoundTripAllItems)
{
    for (const bool rise_manual : {false, true}) {
        for (const bool fall_manual : {false, true}) {
            const Motor tx = make_configured_motor(rise_manual, fall_manual);
            Motor       rx;

            // CAN 2.0 (8バイト) では、どの組み合わせでも4フレーム以内
            const std::size_t frames = send_all_config(tx, rx, 8);
            EXPECT_LE(frames, 4U) << rise_manual << fall_manual;

            expect_same_motor(
                tx, rx, "rise_manual=" + std::to_string(rise_manual) + " fall_manual=" + std::to_string(fall_manual));
        }
    }
}

TEST(Protocol, ConfigFitsInOneFrameForLargeFrames)
{
    const Motor tx = make_configured_motor(true, true);
    Motor       rx;
    EXPECT_EQ(send_all_config(tx, rx, 64), 1U);
    expect_same_motor(tx, rx);
}

/// 全ての ChangeLevel と、既定値、 PWM 周波数の端でも往復できる
TEST(Protocol, ConfigRoundTripBoundaries)
{
    for (const auto level : {Motor::ChangeLevel::OFF, Motor::ChangeLevel::Low, Motor::ChangeLevel::Middle,
                             Motor::ChangeLevel::High, Motor::ChangeLevel::Max}) {
        Motor tx;
        tx.pid_gain_factor(0.5F, 0.25F, 0.125F);  // bfloat16 で誤差なく表せる値 (既定値は表せない)
        tx.change_level(Motor::ChangeLevelTarget::Rise, level);
        tx.change_level(Motor::ChangeLevelTarget::Fall, level);
        Motor rx;
        send_all_config(tx, rx, 8);
        expect_same_motor(tx, rx);
    }

    // 既定値 (既定のゲインは bfloat16 で表せないので、送受信後も一致するように bfloat16 で表せる値にしてから比較する)
    {
        Motor tx;
        Motor rx;
        send_all_config(tx, rx, 8);
        float kp, ki, kd, rkp, rki, rkd;
        tx.get_pid_gain_factor(kp, ki, kd);
        rx.get_pid_gain_factor(rkp, rki, rkd);
        EXPECT_NEAR(kp, rkp, kp / 128.0F);
        EXPECT_NEAR(ki, rki, ki / 128.0F);
        EXPECT_NEAR(kd, rkd, kd / 128.0F);
        EXPECT_EQ(tx.get_pulse_period(), rx.get_pulse_period());  // 1/5000 は整数Hzなので往復しても同じ
        EXPECT_EQ(tx.get_release_time(), rx.get_release_time());
    }

    for (const float hz : {10.0F, 60.0F, 5000.0F, 60000.0F}) {
        Motor tx;
        tx.pulse_period(1.0F / hz);
        Motor rx;
        send_all_config(tx, rx, 8);
        EXPECT_EQ(tx.get_pulse_period(), rx.get_pulse_period()) << hz;
    }

    for (const float seconds : {0.0F, 0.001F, 1.0F, 65.535F}) {
        Motor tx;
        tx.release_time(seconds);
        tx.full_scale_time(Motor::ChangeLevelTarget::Rise, seconds);
        Motor rx;
        send_all_config(tx, rx, 8);
        EXPECT_NEAR(tx.get_release_time(), rx.get_release_time(), 0.0005F) << seconds;
        EXPECT_NEAR(tx.get_full_scale_time(Motor::ChangeLevelTarget::Rise),
                    rx.get_full_scale_time(Motor::ChangeLevelTarget::Rise), 0.0005F)
            << seconds;
    }
}

TEST(Protocol, ConfigClampsLongTimes)
{
    Motor tx;
    tx.release_time(100.0F);

    testing::internal::CaptureStderr();
    Error::get_instance().reset();
    Motor rx;
    send_all_config(tx, rx, 8);
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Warning);
    testing::internal::GetCapturedStderr();

    EXPECT_FLOAT_EQ(rx.get_release_time(), 65.535F);
}

TEST(Protocol, EncodeConfigFailures)
{
    const Motor motor;
    uint8_t     buffer[8];
    std::size_t bytes = 99, next_index = 99;

    testing::internal::CaptureStderr();
    EXPECT_FALSE(protocol::encode_config(motor, protocol::config_item_count, buffer, 8, bytes, next_index));
    EXPECT_FALSE(protocol::encode_config(motor, 0, nullptr, 8, bytes, next_index));
    EXPECT_FALSE(protocol::encode_config(motor, 0, buffer, 0, bytes, next_index));
    // pid_gain (63bit) は 7バイトには入らない
    EXPECT_FALSE(protocol::encode_config(motor, 0, buffer, 7, bytes, next_index));
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(bytes, 99U);
    EXPECT_EQ(next_index, 99U);
}

/// 設定は制御パケットではない (release_time のタイマーをリセットしない)
TEST(Protocol, ConfigIsNotControl)
{
    uint8_t     buffer[8];
    std::size_t bytes = 0, next_index = 0;
    Motor       motor;
    ASSERT_TRUE(protocol::encode_config(motor, 0, buffer, sizeof(buffer), bytes, next_index));
    EXPECT_FALSE(protocol::decode(buffer, bytes, motor).is_control());
}

// ---------------------------------------------------------------------------
// 前方互換 (未知の項目)
// ---------------------------------------------------------------------------

TEST(Protocol, UnknownItemsAreSkipped)
{
    std::mt19937 rng(12345);

    for (int iteration = 0; iteration < 2000; iteration++) {
        uint8_t   buffer[64];
        BitWriter writer(buffer, sizeof(buffer));
        writer.write(0b110, 3);

        // 既知の項目 (decay=Fast、 sleep=true、 pwm_side=High) と、未知の項目を、並べ替えて並べる
        struct Entry {
            bool     known;
            uint32_t kind;
            uint32_t length;
            uint32_t value;
        };
        std::vector<Entry> entries = {{true, 4, 2, 2}, {true, 7, 1, 1}, {true, 5, 1, 1}};
        const int          unknown = static_cast<int>(rng() % 5);
        for (int i = 0; i < unknown; i++) {
            const uint32_t kind = 12 + (rng() % 8 == 0 ? (rng() % (1U << 20)) : (rng() % 100));
            entries.push_back({false, kind, static_cast<uint32_t>(rng() % 33), static_cast<uint32_t>(rng())});
        }
        std::shuffle(entries.begin(), entries.end(), rng);

        for (const auto& e : entries) {
            ASSERT_TRUE(writer.write_evi(e.kind));
            ASSERT_TRUE(writer.write_evi(e.length));
            ASSERT_TRUE(writer.write(e.value, e.length));
        }

        Motor      motor;
        const auto result = protocol::decode(buffer, writer.bytes(), motor);
        ASSERT_EQ(result.status, DecodeStatus::Applied) << iteration;
        EXPECT_EQ(result.applied_items, 3);
        EXPECT_EQ(result.skipped_items, unknown);
        EXPECT_EQ(motor.get_decay(), Motor::Decay::Fast);
        EXPECT_TRUE(motor.get_sleep());
        EXPECT_EQ(motor.get_pwm_side(), Motor::PwmSide::High);
    }
}

/// 同じ種類が重なったときは、後のものが勝つ
TEST(Protocol, LaterItemWins)
{
    uint8_t   buffer[8];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b110, 3);
    for (const uint32_t decay : {2U, 0U}) {  // Fast の後に Slow
        writer.write_evi(4);
        writer.write_evi(2);
        writer.write(decay, 2);
    }

    Motor motor;
    EXPECT_EQ(protocol::decode(buffer, writer.bytes(), motor).status, DecodeStatus::Applied);
    EXPECT_EQ(motor.get_decay(), Motor::Decay::Slow);
}

// ---------------------------------------------------------------------------
// 検証規則 (予約値・範囲外)
// ---------------------------------------------------------------------------

/// 1項目だけの Config を作って decode し、結果と Motor が変わらないことを確認する
DecodeStatus decode_single_item(const uint32_t kind, const uint32_t length, const uint32_t value, Motor& motor)
{
    uint8_t   buffer[8];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b110, 3);
    writer.write_evi(kind);
    writer.write_evi(length);
    writer.write(value, length);
    return protocol::decode(buffer, writer.bytes(), motor).status;
}

TEST(Protocol, InvalidItemValuesAreRejected)
{
    struct Case {
        uint32_t    kind, length, value;
        const char* name;
    };
    const Case cases[] = {
        {2, 16, 9, "pwm_frequency=9"},
        {2, 16, 60001, "pwm_frequency=60001"},
        {2, 16, 0, "pwm_frequency=0"},
        {4, 2, 1, "decay=Mixed"},
        {4, 2, 3, "decay=3"},
        {8, 3, 0, "rise_change_level=Manual"},
        {8, 3, 6, "rise_change_level=6"},
        {9, 3, 7, "fall_change_level=7"},
    };

    for (const auto& c : cases) {
        Motor       motor;
        const Motor before = motor;
        EXPECT_EQ(decode_single_item(c.kind, c.length, c.value, motor), DecodeStatus::InvalidValue) << c.name;
        expect_same_motor(before, motor, c.name);
    }

    // 範囲の端は受け付ける
    for (const uint32_t hz : {10U, 60000U}) {
        Motor motor;
        EXPECT_EQ(decode_single_item(2, 16, hz, motor), DecodeStatus::Applied) << hz;
    }
}

TEST(Protocol, InvalidGainsAreRejected)
{
    // 負、 +inf、 NaN の bfloat16
    for (const uint16_t bad : {0xBF80 /* -1.0 */, 0x7F80, 0x7FC0}) {
        for (int position = 0; position < 3; position++) {
            uint8_t   buffer[8];
            BitWriter writer(buffer, sizeof(buffer));
            writer.write(0b110, 3);
            writer.write_evi(1);
            writer.write_evi(48);
            for (int i = 0; i < 3; i++) {
                writer.write((i == position) ? bad : 0x3F80, 16);
            }

            Motor       motor;
            const Motor before = motor;
            EXPECT_EQ(protocol::decode(buffer, writer.bytes(), motor).status, DecodeStatus::InvalidValue);
            expect_same_motor(before, motor);
        }
    }
}

/// 同じフレームの中に不正な項目があれば、先に出てきた正しい項目も適用しない (原子性)
TEST(Protocol, FrameIsAtomic)
{
    uint8_t   buffer[8];
    BitWriter writer(buffer, sizeof(buffer));
    writer.write(0b110, 3);
    writer.write_evi(4);  // decay=Fast (正しい)
    writer.write_evi(2);
    writer.write(2, 2);
    writer.write_evi(4);  // decay=Mixed (不正)
    writer.write_evi(2);
    writer.write(1, 2);

    Motor motor;
    EXPECT_EQ(protocol::decode(buffer, writer.bytes(), motor).status, DecodeStatus::InvalidValue);
    EXPECT_EQ(motor.get_decay(), Motor::Decay::Slow);

    // 固定フィールドが正しくても、後ろの項目列が壊れていれば、 PWM の値も適用しない
    Bytes data = {0x30, 0x00, 0x08, 0x50};  // PWM CW 50% + decay=Fast
    data.push_back(0xFF);                   // 壊れた項目
    Motor pwm;
    EXPECT_EQ(decode(data, pwm).status, DecodeStatus::Malformed);
    EXPECT_EQ(pwm.get_state(), Motor::State::Brake);
    EXPECT_EQ(pwm.get_decay(), Motor::Decay::Slow);
}

TEST(Protocol, MalformedLengths)
{
    Motor       motor;
    const Motor before = motor;

    // 既知の項目の長さが ±1 違う
    for (const uint32_t length : {1U, 3U}) {
        EXPECT_EQ(decode_single_item(4, length, 0, motor), DecodeStatus::Malformed);
    }
    for (const uint32_t length : {15U, 17U}) {
        EXPECT_EQ(decode_single_item(3, length, 0, motor), DecodeStatus::Malformed);
    }
    EXPECT_EQ(decode_single_item(1, 47, 0, motor), DecodeStatus::Malformed);

    // 長さが残りより長い (未知の種類 12、長さ 5 なのに、値が 2bit しかない)
    {
        uint8_t   buffer[2];
        BitWriter writer(buffer, sizeof(buffer));
        writer.write(0b110, 3);
        writer.write_evi(12);
        writer.write_evi(5);
        writer.write(0b11, 2);
        // 値が足りないので、詰め物の扱いにはならず Malformed
        EXPECT_EQ(protocol::decode(buffer, 2, motor).status, DecodeStatus::Malformed);
        expect_same_motor(before, motor);
    }

    // 型だけの Config は有効で、何も変えない
    EXPECT_EQ(decode({0xC0}, motor).status, DecodeStatus::Applied);
    expect_same_motor(before, motor);
}

// ---------------------------------------------------------------------------
// ファズ
// ---------------------------------------------------------------------------

/// どんなデータを与えても、クラッシュせず、 Error を通知せず、 Applied 以外では Motor を変更しない
void fuzz_one(const Bytes& data, std::size_t& applied)
{
    Motor motor;
    motor.duty_cycle(0.4F);
    motor.speed(3.0F);
    const Motor before = motor;

    Error::get_instance().reset();
    const auto result = protocol::decode(data.empty() ? nullptr : data.data(), data.size(), motor);
    ASSERT_EQ(Error::get_instance().get_status(), Error::Status::Normal);

    if (result.status != DecodeStatus::Applied) {
        expect_same_motor(before, motor);
        EXPECT_FALSE(result.is_control());
        return;
    }

    applied++;

    // Applied のときは、 Motor が不変条件を満たしている
    EXPECT_GE(motor.get_duty_cycle(), 0.0F);
    EXPECT_LE(motor.get_duty_cycle(), 1.0F);
    EXPECT_TRUE(std::isfinite(motor.get_speed()));
    EXPECT_GE(motor.get_speed(), 0.0F);
    // C++14 では static constexpr メンバーを参照渡し(EXPECT_GE など)すると未定義参照になるため、コピーを使う
    const float min_pulse_period = Motor::min_pulse_period;
    const float max_pulse_period = Motor::max_pulse_period;
    EXPECT_GE(motor.get_pulse_period(), min_pulse_period);
    EXPECT_LE(motor.get_pulse_period(), max_pulse_period);
    EXPECT_NE(motor.get_decay(), Motor::Decay::Mixed);

    // 再 encode して decode しても、同じ状態になる (制御パケットのとき)
    if (result.is_control()) {
        uint8_t     buffer[8];
        std::size_t bytes = 0;
        ASSERT_TRUE(protocol::encode_control(motor, buffer, sizeof(buffer), bytes));
        Motor again;
        ASSERT_EQ(protocol::decode(buffer, bytes, again).status, DecodeStatus::Applied);
        EXPECT_EQ(again.get_state(), motor.get_state());
        EXPECT_EQ(again.get_control_system(), motor.get_control_system());
    }
}

TEST(Protocol, FuzzRandomBytes)
{
    std::mt19937 rng(2024);
    std::size_t  applied = 0;

    for (int i = 0; i < 100000; i++) {
        Bytes data(rng() % 65);
        for (auto& b : data) {
            // 0 を多めにして、詰め物や短い項目の経路を通りやすくする
            b = (rng() % 4 == 0) ? 0x00 : static_cast<uint8_t>(rng());
        }
        fuzz_one(data, applied);
    }

    // 少なくとも一部は Applied になる (ファズが拒否ばかりで終わっていないことの確認)
    EXPECT_GT(applied, 100U);
}

/// 正しいデータの1bitを反転させたもの
TEST(Protocol, FuzzBitFlips)
{
    std::vector<Bytes> seeds   = {{0x30, 0x00, 0x00},
                                  {0xA4, 0x14, 0x80},
                                  {0xC8, 0x50},
                                  {0xC5, 0x40, 0x9C, 0x40, 0x74, 0x00, 0x3E, 0x80},
                                  {0xDC, 0x4B, 0x54, 0x28},
                                  {0xC3, 0xC0, 0x7F, 0x80, 0x7D, 0x00, 0x7C, 0x00}};
    std::size_t        applied = 0;
    for (const auto& seed : seeds) {
        for (std::size_t bit = 0; bit < seed.size() * 8; bit++) {
            Bytes data = seed;
            data[bit / 8] ^= static_cast<uint8_t>(0x80 >> (bit % 8));
            fuzz_one(data, applied);
        }
        // 末尾を削ったもの
        for (std::size_t length = 0; length < seed.size(); length++) {
            fuzz_one(Bytes(seed.begin(), seed.begin() + length), applied);
        }
    }
}

}  // namespace
