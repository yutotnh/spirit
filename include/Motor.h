#ifndef SPIRIT_MOTOR_H
#define SPIRIT_MOTOR_H

#include <cstdint>

namespace spirit {

/** @brief モーターをあらわすクラス
 */
class Motor {
public:
    /**
     * @enum State
     * @brief モーターの回転方向を設定すための値
     */
    enum class State {
        /// 空転(緩やかに減速)
        Coast,
        /// 時計回り(Clock Wise)
        CW,
        /// 反時計回り(Counter Clock Wise)
        CCW,
        /// ブレーキ
        Brake,
    };

    /**
     * @enum ChangeLevelTarget
     * @brief ChangeLevel を設定したい側を設定するための値
     */
    enum class ChangeLevelTarget {
        /// 上昇時
        Rise,
        /// 下降時
        Fall,
    };

    /**
     * @enum ChangeLevel
     * @brief モータのデューティー比の立ち上がり・立ち下り時の変化具合を設定するための値
     */
    enum class ChangeLevel {
        /// 設定したデューティー比を直ちにモーターに出力する
        OFF,
        /// ほんの少し余裕を持たせてモーターに出力する
        Low,
        /// 少し余裕を持たせてモーターに出力する
        Middle,
        /// 余裕を持たせてモーターに出力する
        High,
        /// かなり余裕を持たせてモーターに出力する
        Max
    };

    /**
     * @enum Decay
     * @brief モーターの制御方法を設定するための値
     */
    enum class Decay {
        /// Slow decay
        Slow,
        /// Mixed decay(未対応)
        Mixed,
        /// Fast decay
        Fast,
    };

    /**
     * @enum PwmSide
     * @brief Slow decayのPWM sideを設定するための値
     */
    enum class PwmSide : uint8_t {
        /// Low side
        Low,
        /// High side
        High,
    };

    /**
     * @brief デューティー比を設定する
     * @param value
     * 設定したいデューティー比
     * - 0.00F = 0%, 1.00F = 100%
     * - 範囲: 0.00F <= value <= 1.00F
     * - value < 0.00F の場合は 0.00F に設定される
     * - 1.00F < value の場合は 1.00F に設定される
     */
    void duty_cycle(float value);

    /**
     * @brief デューティー比を返す
     * @param value 設定したデューティー比
     *         - 0.00F = 0%, 1.00F = 100%
     *         - 範囲: 0.00F <= value <= 1.00F
     */
    float get_duty_cycle() const;

    /**
     * @brief 速度(rps)を設定する
     * @param rps 設定したい速度(rps)
     */
    void velocity(float rps);

    /**
     * @brief 速度を返す
     * @return 設定した速度(rps)
     */
    float get_velocity() const;

    /**
     * @brief 回転方向を設定する
     * @param type State::Coast, State::CW, State::CCW, State::Brake のいずれかの設定したい回転方向
     */
    void state(State type);

    /**
     * @brief 回転方向を返す
     * @return 設定した回転方向
     */
    State get_state() const;

    /**
     * @brief デューティ比の変化具合を設定する ChangeLevelTarget
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @param level ChangeLevel::OFF, ChangeLevel::Low, ChangeLevel::Middle, ChangeLevel::High, ChangeLevel::Max のいずれかの設定したい変化具合
     */
    void change_level(ChangeLevelTarget target, ChangeLevel level);

    /**
     * @brief デューティ比の変化具合を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @return 設定したデューティ比の変化具合
     */
    ChangeLevel get_change_level(ChangeLevelTarget target) const;

    /**
     * @brief パルス周期を設定する
     * @param seconds
     * 設定したいパルス周期(s)
     * - 範囲: 0.1s <= seconds <= 1.6666e-5s
     */
    void pulse_period(float seconds);

    /**
     * @brief パルス周期を返す
     * @return 設定したパルス周期(s)
     */
    float get_pulse_period() const;

    /**
     * @brief 最後にデータが来て何秒でモータを停止するかを設定する
     * @param seconds 停止までの秒数
     */
    void release_time(float seconds);

    /**
     * @brief 最後にデータが来てから停止までの秒数を返す
     * @return 設定した停止までの秒数
     */
    float get_release_time() const;

    /**
     * @brief Decayを設定する
     * @param type Decay::Slow, Decay::Fast のいずれか
     */
    void decay(Decay type);

    /**
     * @brief Decayを返す
     * @return 設定したDecay
     */
    Decay get_decay() const;

    /**
     * @brief Slow decayのPWM sideを設定する
     * @param type PwmSide::Low, PwmSide::High のいずれか
     */
    void pwm_side(PwmSide type);

    /**
     * @brief Slow decayのPWM sideを返す
     * @return 設定したPwmSide
     */
    PwmSide pwm_side() const;

    /**
     * @brief リセット機能を実行する
     *      - 初期状態: 無効
     * @param enabled リセット機能を有効/無効
     *              - true: 有効
     *              - false: 無効
     */
    void reset(bool enabled);

    /**
     * @brief リセット機能の状態を返す
     * @retval true リセット機能が有効
     * @retval false リセット機能が無効
     */
    bool get_reset() const;

    /**
     * @brief スリープ機能を有効にする
     *      - 初期状態: 無効
     * @param enabled
     *      - true : スリープモードを有効にする
     *      - false: スリープモードを解除する
     */
    void sleep(bool enabled);

    /**
     * @brief スリープ機能の状態を返す
     * @retval true スリープモードが有効
     * @retval false スリープモードが無効
     */
    bool get_sleep() const;

    static constexpr State       default_state             = State::Brake;
    static constexpr ChangeLevel default_rise_change_level = ChangeLevel::OFF;
    static constexpr ChangeLevel default_fall_change_level = ChangeLevel::OFF;
    static constexpr float       default_pulse_period      = 1.0F / 5'000.0F;
    static constexpr float       min_pulse_period          = 1.0F / 60'000.0F;
    static constexpr float       max_pulse_period          = 1.0F / 10.0F;
    static constexpr Decay       default_decay             = Decay::Slow;
    static constexpr PwmSide     default_pwm_side          = PwmSide::Low;
    static constexpr bool        default_reset             = false;
    static constexpr bool        default_sleep             = false;

private:
    float       _duty_cycle{0.0F};
    float       _velocity{0.0F};
    State       _state{default_state};
    ChangeLevel _rise_change_level{default_rise_change_level};
    ChangeLevel _fall_change_level{default_fall_change_level};
    float       _pulse_period{default_pulse_period};
    float       _release_time{0.0F};
    Decay       _decay{default_decay};
    PwmSide     _pwm_side{default_pwm_side};
    bool        _reset{default_reset};
    bool        _sleep{default_sleep};
};

}  // namespace spirit

#endif  // SPIRIT_MOTOR_H
