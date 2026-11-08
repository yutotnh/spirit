#include "spirit/include/Protocol.h"

#include <cmath>

#include "spirit/include/BitStream.h"
#include "spirit/include/Error.h"
#include "spirit/include/bfloat16.h"

namespace spirit {

namespace protocol {

namespace {

/// 制御パケットのバイト数
constexpr std::size_t control_bytes = 3;

/// 設定パケットの型 (3bit) のビット数
constexpr std::size_t config_header_bits = 3;

/// 項目の種類
enum Kind : uint32_t {
    KindPidGain         = 1,
    KindPwmFrequency    = 2,
    KindReleaseTime     = 3,
    KindDecay           = 4,
    KindPwmSide         = 5,
    KindReset           = 6,
    KindSleep           = 7,
    KindRiseChangeLevel = 8,
    KindFallChangeLevel = 9,
    KindRiseFullScaleMs = 10,
    KindFallFullScaleMs = 11,
};

constexpr uint32_t max_uint16 = 65535;

/// PWM周波数の範囲 (Motor::min_pulse_period, Motor::max_pulse_period に対応する)
constexpr uint32_t min_pwm_frequency = 10;
constexpr uint32_t max_pwm_frequency = 60000;

bool state_to_wire(const Motor::State state, uint32_t& wire)
{
    switch (state) {
        case Motor::State::Coast:
            wire = 0;
            return true;
        case Motor::State::CW:
            wire = 1;
            return true;
        case Motor::State::CCW:
            wire = 2;
            return true;
        case Motor::State::Brake:
            wire = 3;
            return true;
        default:
            return false;
    }
}

Motor::State wire_to_state(const uint32_t wire)
{
    switch (wire & 0x3U) {
        case 0:
            return Motor::State::Coast;
        case 1:
            return Motor::State::CW;
        case 2:
            return Motor::State::CCW;
        default:
            return Motor::State::Brake;
    }
}

/// 秒をミリ秒に変換する。 uint16 に収まらない場合は収めて、 warning を通知する
uint32_t seconds_to_milliseconds(const float seconds)
{
    const float milliseconds = std::floor(seconds * 1000.0F + 0.5F);

    if (!(milliseconds <= static_cast<float>(max_uint16))) {
        Error::get_instance().warning(Error::Type::InvalidValue, __FILE__, __func__, __LINE__,
                                      "Time (%g s) exceeds the maximum value (65.535 s), so it will be 65.535 s",
                                      seconds);
        return max_uint16;
    }

    if (!(0.0F <= milliseconds)) {
        return 0;
    }

    return static_cast<uint32_t>(milliseconds);
}

/// 送信する1項目
struct Item {
    uint32_t kind;
    /// 値のビット数
    uint32_t length;
    /// 値 (pid_gain 以外は values[0] だけを使う。 pid_gain は3つの値を16bitずつ使う)
    uint32_t values[3];

    std::size_t bits() const
    {
        return BitWriter::evi_bits(kind) + BitWriter::evi_bits(length) + length;
    }
};

bool change_level_to_wire(const Motor::ChangeLevel level, uint32_t& wire)
{
    switch (level) {
        case Motor::ChangeLevel::OFF:
            wire = 1;
            return true;
        case Motor::ChangeLevel::Low:
            wire = 2;
            return true;
        case Motor::ChangeLevel::Middle:
            wire = 3;
            return true;
        case Motor::ChangeLevel::High:
            wire = 4;
            return true;
        case Motor::ChangeLevel::Max:
            wire = 5;
            return true;
        default:
            return false;
    }
}

bool wire_to_change_level(const uint32_t wire, Motor::ChangeLevel& level)
{
    switch (wire) {
        case 1:
            level = Motor::ChangeLevel::OFF;
            return true;
        case 2:
            level = Motor::ChangeLevel::Low;
            return true;
        case 3:
            level = Motor::ChangeLevel::Middle;
            return true;
        case 4:
            level = Motor::ChangeLevel::High;
            return true;
        case 5:
            level = Motor::ChangeLevel::Max;
            return true;
        default:
            return false;
    }
}

/// Rise/Fall の項目 (change_level または full_scale_time) を作る
bool make_change_item(const Motor& motor, const Motor::ChangeLevelTarget target, Item& item)
{
    const bool rise = (target == Motor::ChangeLevelTarget::Rise);

    if (motor.get_change_level(target) == Motor::ChangeLevel::Manual) {
        item.kind      = rise ? KindRiseFullScaleMs : KindFallFullScaleMs;
        item.length    = 16;
        item.values[0] = seconds_to_milliseconds(motor.get_full_scale_time(target));
        return true;
    }

    item.kind   = rise ? KindRiseChangeLevel : KindFallChangeLevel;
    item.length = 3;
    return change_level_to_wire(motor.get_change_level(target), item.values[0]);
}

/// 送信順 (PROTOCOL.md 6.4 節) の slot 番目の項目を作る
bool make_item(const Motor& motor, const std::size_t slot, Item& item)
{
    item.values[0] = item.values[1] = item.values[2] = 0;

    switch (slot) {
        case 0: {
            float kp = 0.0F, ki = 0.0F, kd = 0.0F;
            motor.get_pid_gain_factor(kp, ki, kd);
            item.kind      = KindPidGain;
            item.length    = 48;
            item.values[0] = float32_to_bfloat16(kp);
            item.values[1] = float32_to_bfloat16(ki);
            item.values[2] = float32_to_bfloat16(kd);
            return true;
        }
        case 1: {
            const float frequency = std::floor(1.0F / motor.get_pulse_period() + 0.5F);
            uint32_t    hz        = max_pwm_frequency;
            if (frequency < static_cast<float>(min_pwm_frequency)) {
                hz = min_pwm_frequency;
            } else if (frequency < static_cast<float>(max_pwm_frequency)) {
                hz = static_cast<uint32_t>(frequency);
            }
            item.kind      = KindPwmFrequency;
            item.length    = 16;
            item.values[0] = hz;
            return true;
        }
        case 2:
            item.kind      = KindReleaseTime;
            item.length    = 16;
            item.values[0] = seconds_to_milliseconds(motor.get_release_time());
            return true;
        case 3:
            return make_change_item(motor, Motor::ChangeLevelTarget::Rise, item);
        case 4:
            item.kind   = KindDecay;
            item.length = 2;
            switch (motor.get_decay()) {
                case Motor::Decay::Slow:
                    item.values[0] = 0;
                    return true;
                case Motor::Decay::Mixed:
                    item.values[0] = 1;
                    return true;
                case Motor::Decay::Fast:
                    item.values[0] = 2;
                    return true;
                default:
                    return false;
            }
        case 5:
            item.kind   = KindPwmSide;
            item.length = 1;
            switch (motor.get_pwm_side()) {
                case Motor::PwmSide::Low:
                    item.values[0] = 0;
                    return true;
                case Motor::PwmSide::High:
                    item.values[0] = 1;
                    return true;
                default:
                    return false;
            }
        case 6:
            item.kind      = KindReset;
            item.length    = 1;
            item.values[0] = motor.get_reset() ? 1 : 0;
            return true;
        case 7:
            return make_change_item(motor, Motor::ChangeLevelTarget::Fall, item);
        case 8:
            item.kind      = KindSleep;
            item.length    = 1;
            item.values[0] = motor.get_sleep() ? 1 : 0;
            return true;
        default:
            return false;
    }
}

bool write_item(BitWriter& writer, const Item& item)
{
    if (!writer.write_evi(item.kind) || !writer.write_evi(item.length)) {
        return false;  // LCOV_EXCL_LINE
    }

    if (item.length == 48) {
        return writer.write(item.values[0], 16) && writer.write(item.values[1], 16) && writer.write(item.values[2], 16);
    }

    return writer.write(item.values[0], item.length);
}

/// 項目の検査結果
enum class ItemResult : uint8_t { Applied, Skipped, Malformed, InvalidValue };

/// 読み出した pid_gain の値が正しいか (有限で0以上)
bool valid_gain(const float gain)
{
    return std::isfinite(gain) && (0.0F <= gain);
}

/// 項目を1つ読み、検査して、 next に適用する (next は Motor のコピー)
ItemResult parse_item(BitReader& reader, Motor& next)
{
    uint32_t kind   = 0;
    uint32_t length = 0;

    if (!reader.read_evi(kind) || !reader.read_evi(length)) {
        return ItemResult::Malformed;
    }

    // 種類0は禁止 (詰め物と区別できない)
    if (kind == 0) {
        return ItemResult::Malformed;
    }

    if (reader.remaining_bits() < length) {
        return ItemResult::Malformed;
    }

    // 既知の種類の、値の長さ (ビット)
    uint32_t expected = 0;
    switch (kind) {
        case KindPidGain:
            expected = 48;
            break;
        case KindPwmFrequency:
        case KindReleaseTime:
        case KindRiseFullScaleMs:
        case KindFallFullScaleMs:
            expected = 16;
            break;
        case KindDecay:
            expected = 2;
            break;
        case KindPwmSide:
        case KindReset:
        case KindSleep:
            expected = 1;
            break;
        case KindRiseChangeLevel:
        case KindFallChangeLevel:
            expected = 3;
            break;
        default:
            // 未知の項目は、長さの分だけ読み飛ばす
            return reader.skip(length) ? ItemResult::Skipped : ItemResult::Malformed;
    }

    if (length != expected) {
        return ItemResult::Malformed;
    }

    uint32_t value = 0;

    switch (kind) {
        case KindPidGain: {
            uint32_t raw[3] = {0, 0, 0};
            if (!reader.read(16, raw[0]) || !reader.read(16, raw[1]) || !reader.read(16, raw[2])) {
                return ItemResult::Malformed;  // LCOV_EXCL_LINE
            }
            const float kp = bfloat16_to_float32(static_cast<uint16_t>(raw[0]));
            const float ki = bfloat16_to_float32(static_cast<uint16_t>(raw[1]));
            const float kd = bfloat16_to_float32(static_cast<uint16_t>(raw[2]));
            if (!valid_gain(kp) || !valid_gain(ki) || !valid_gain(kd)) {
                return ItemResult::InvalidValue;
            }
            next.pid_gain_factor(kp, ki, kd);
            return ItemResult::Applied;
        }
        default:
            break;
    }

    if (!reader.read(length, value)) {
        return ItemResult::Malformed;  // LCOV_EXCL_LINE
    }

    switch (kind) {
        case KindPwmFrequency:
            if ((value < min_pwm_frequency) || (max_pwm_frequency < value)) {
                return ItemResult::InvalidValue;
            }
            next.pulse_period(1.0F / static_cast<float>(value));
            return ItemResult::Applied;
        case KindReleaseTime:
            next.release_time(static_cast<float>(value) / 1000.0F);
            return ItemResult::Applied;
        case KindDecay:
            // 1 は Mixed (未対応)、 3 は予約
            if (value == 0) {
                next.decay(Motor::Decay::Slow);
            } else if (value == 2) {
                next.decay(Motor::Decay::Fast);
            } else {
                return ItemResult::InvalidValue;
            }
            return ItemResult::Applied;
        case KindPwmSide:
            next.pwm_side((value == 0) ? Motor::PwmSide::Low : Motor::PwmSide::High);
            return ItemResult::Applied;
        case KindReset:
            next.reset(value != 0);
            return ItemResult::Applied;
        case KindSleep:
            next.sleep(value != 0);
            return ItemResult::Applied;
        case KindRiseChangeLevel:
        case KindFallChangeLevel: {
            Motor::ChangeLevel level = Motor::ChangeLevel::OFF;
            if (!wire_to_change_level(value, level)) {
                return ItemResult::InvalidValue;
            }
            next.change_level(
                (kind == KindRiseChangeLevel) ? Motor::ChangeLevelTarget::Rise : Motor::ChangeLevelTarget::Fall, level);
            return ItemResult::Applied;
        }
        case KindRiseFullScaleMs:
        case KindFallFullScaleMs:
            next.full_scale_time(
                (kind == KindRiseFullScaleMs) ? Motor::ChangeLevelTarget::Rise : Motor::ChangeLevelTarget::Fall,
                static_cast<float>(value) / 1000.0F);
            return ItemResult::Applied;
        default:
            return ItemResult::Malformed;  // LCOV_EXCL_LINE
    }
}

/// 項目列を最後まで読み、 next に適用する
DecodeStatus parse_items(BitReader& reader, Motor& next, DecodeResult& result)
{
    // 項目は必ず1のビットから始まる (種類が1以上なので) ため、残りが全て0なら詰め物
    while (!reader.remaining_is_zero()) {
        switch (parse_item(reader, next)) {
            case ItemResult::Applied:
                if (result.applied_items < UINT8_MAX) {
                    result.applied_items++;
                }
                break;
            case ItemResult::Skipped:
                if (result.skipped_items < UINT8_MAX) {
                    result.skipped_items++;
                }
                break;
            case ItemResult::Malformed:
                return DecodeStatus::Malformed;
            case ItemResult::InvalidValue:
                return DecodeStatus::InvalidValue;
        }
    }

    return DecodeStatus::Applied;
}

}  // namespace

bool encode_control(const Motor& motor, uint8_t* buffer, const std::size_t max_bytes, std::size_t& bytes)
{
    if ((buffer == nullptr) || (max_bytes < control_bytes)) {
        Error::get_instance().warning(Error::Type::IllegalCombination, __FILE__, __func__, __LINE__,
                                      "Buffer size (%zu bytes) is less than the control packet size (%zu bytes)",
                                      max_bytes, control_bytes);
        return false;
    }

    uint32_t state = 0;
    if (!state_to_wire(motor.get_state(), state)) {
        Error::get_instance().warning(Error::Type::UnknownValue, __FILE__, __func__, __LINE__,
                                      "Unknown motor state (%d)", static_cast<uint32_t>(motor.get_state()));
        return false;
    }

    BitWriter writer(buffer, max_bytes);

    switch (motor.get_control_system()) {
        case Motor::ControlSystem::PWM: {
            // NaN は 0 として扱い、範囲外は収める
            float duty_cycle = motor.get_duty_cycle();
            if (!(0.0F < duty_cycle)) {
                duty_cycle = 0.0F;
            } else if (1.0F < duty_cycle) {
                duty_cycle = 1.0F;
            }
            const uint32_t duty_cycle_16bit = static_cast<uint32_t>(65535.0F * duty_cycle + 0.5F);

            writer.write(0b0, 1);
            writer.write(state, 2);
            writer.write(duty_cycle_16bit, 16);
            break;
        }
        case Motor::ControlSystem::Speed:
            writer.write(0b10, 2);
            writer.write(state, 2);
            writer.write(float32_to_bfloat16(motor.get_speed()), 16);
            break;
        default:
            Error::get_instance().warning(Error::Type::UnknownValue, __FILE__, __func__, __LINE__,
                                          "Unknown motor control system (%d)",
                                          static_cast<uint32_t>(motor.get_control_system()));
            return false;
    }

    bytes = control_bytes;
    return true;
}

bool encode_config(const Motor& motor, const std::size_t index, uint8_t* buffer, const std::size_t max_bytes,
                   std::size_t& bytes, std::size_t& next_index)
{
    if ((buffer == nullptr) || (max_bytes == 0) || (config_item_count <= index)) {
        Error::get_instance().warning(Error::Type::IllegalCombination, __FILE__, __func__, __LINE__,
                                      "Invalid arguments (index = %zu, max_bytes = %zu)", index, max_bytes);
        return false;
    }

    BitWriter writer(buffer, max_bytes);
    writer.write(0b110, config_header_bits);

    std::size_t slot = index;
    while (slot < config_item_count) {
        Item item;
        if (!make_item(motor, slot, item)) {
            Error::get_instance().warning(Error::Type::UnknownValue, __FILE__, __func__, __LINE__,
                                          "Unknown motor setting (slot = %zu)", slot);
            return false;
        }

        // 入りきらない項目は、次のフレームに回す (順番を飛ばして詰めることはしない)
        if (writer.remaining_bits() < item.bits()) {
            break;
        }

        write_item(writer, item);
        slot++;
    }

    if (slot == index) {
        Error::get_instance().warning(Error::Type::IllegalCombination, __FILE__, __func__, __LINE__,
                                      "Buffer size (%zu bytes) is too small for a setting", max_bytes);
        return false;
    }

    bytes      = writer.bytes();
    next_index = (slot == config_item_count) ? 0 : slot;
    return true;
}

DecodeResult decode(const uint8_t* buffer, const std::size_t bytes, Motor& motor)
{
    DecodeResult result;

    BitReader reader(buffer, bytes);

    uint32_t bit = 0;
    if (!reader.read(1, bit)) {
        result.status = DecodeStatus::Malformed;
        return result;
    }

    // 型 (プレフィックス符号): 0 = PWM, 10 = Speed, 110 = Config, 111... = 予約・拡張
    Motor next = motor;

    if (bit == 0) {
        result.type = MessageType::Pwm;

        uint32_t state = 0, duty_cycle = 0;
        if (!reader.read(2, state) || !reader.read(16, duty_cycle)) {
            result.status = DecodeStatus::Malformed;
            return result;
        }

        next.control_system(Motor::ControlSystem::PWM);
        next.duty_cycle(static_cast<float>(duty_cycle) / 65535.0F);
        next.state(wire_to_state(state));
    } else {
        if (!reader.read(1, bit)) {
            result.status = DecodeStatus::Malformed;
            return result;
        }

        if (bit == 0) {
            result.type = MessageType::Speed;

            uint32_t state = 0, speed_16bit = 0;
            if (!reader.read(2, state) || !reader.read(16, speed_16bit)) {
                result.status = DecodeStatus::Malformed;
                return result;
            }

            // 有限で0以上 (-0 は可)
            const float speed = bfloat16_to_float32(static_cast<uint16_t>(speed_16bit));
            if (!std::isfinite(speed) || !(0.0F <= speed)) {
                result.status = DecodeStatus::InvalidValue;
                return result;
            }

            next.control_system(Motor::ControlSystem::Speed);
            next.speed(speed);
            next.state(wire_to_state(state));
        } else {
            if (!reader.read(1, bit)) {
                result.status = DecodeStatus::Malformed;
                return result;
            }

            if (bit != 0) {
                // 1110 (分割転送の予約) と 1111... (拡張型) は未対応
                result.status = DecodeStatus::UnknownType;
                return result;
            }

            result.type = MessageType::Config;
        }
    }

    result.status = parse_items(reader, next, result);
    if (result.status == DecodeStatus::Applied) {
        motor = next;
    }

    return result;
}

}  // namespace protocol

}  // namespace spirit
