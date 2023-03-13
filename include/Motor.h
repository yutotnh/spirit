#ifndef SPIRIT_MOTOR_H
#define SPIRIT_MOTOR_H

#include <cmath>
#include <cstdint>

namespace spirit {

/** @brief モーターをあらわすクラス
 */
class Motor {
public:
    /**
     * @brief 制御方式を設定するための値
     */
    enum class ControlSystem {
        /// PWM制御
        PWM,
        /// 速度制御
        Speed,
    };

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
        /// 手動で単位当たりの最大変化デューティ比を入力する
        Manual,
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
     * @brief 制御方式を設定する
     * @param type ControlSystem::PWM, ControlSystem::Speed のいずれかの設定したい制御
     */
    void control_system(ControlSystem type);

    /**
     * @brief 制御方式を返す
     * @return ControlSystem::PWM, ControlSystem::Speed のいずれかの設定した制御
     */
    ControlSystem get_control_system() const;

    /**
     * @brief デューティー比を設定する
     * @param value
     * 設定したいデューティー比
     * - 0.00F = 0%, 1.00F = 100%
     * - 範囲: 0.00F <= value <= 1.00F
     * - 範囲を超えた場合は、範囲の端に設定される
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
    void speed(float rps);

    /**
     * @brief 速度を返す
     * @return 設定した速度(rps)
     */
    float get_speed() const;

    /**
     * @brief PID制御のゲイン係数を設定する
     * @param Kp 比例ゲイン係数
     * @param Ki 積分ゲイン係数
     * @param Kd 微分ゲイン係数
     */
    void pid_gain_factor(float Kp, float Ki, float Kd);

    /**
     * @brief PID制御のゲイン係数を返す
     * @param Kp 比例ゲイン係数
     * @param Ki 積分ゲイン係数
     * @param Kd 微分ゲイン係数
     */
    void get_pid_gain_factor(float& Kp, float& Ki, float& Kd) const;

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
     * @param level ChangeLevel::Manual, ChangeLevel::OFF, ChangeLevel::Low, ChangeLevel::Middle, ChangeLevel::High, ChangeLevel::Max のいずれかの設定したい変化具合
     */
    void change_level(ChangeLevelTarget target, ChangeLevel level);

    /**
     * @brief デューティ比の変化具合を設定する ChangeLevelTarget
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @param duty_cycle 単位時間当たりの最大変化デューティ比
     */
    void change_level(ChangeLevelTarget target, float duty_cycle);

    /**
     * @brief デューティ比の変化具合を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @return 設定したデューティ比の変化具合
     */
    ChangeLevel get_change_level(ChangeLevelTarget target) const;

    /**
     * @brief 単位時間当たりの最大変化デューティ比を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @retval duty_cycle 単位時間当たりの最大変化デューティ比
     */
    float get_maximum_change_duty_cycle(ChangeLevelTarget target) const;

    /**
     * @brief パルス周期を設定する
     * @param seconds
     * 設定したいパルス周期(s)
     * - 範囲: 0.1s <= seconds <= 1.6666e-5s
     * - 範囲を超えると、その範囲の端に設定される
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
     *      - 0.0s より小さい場合は、停止しない
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
    PwmSide get_pwm_side() const;

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

    /**
     * @brief デフォルト値を格納している構造体
     */
    struct Default {
        static constexpr ControlSystem control_system            = ControlSystem::PWM;
        static constexpr State         state                     = State::Brake;
        static constexpr ChangeLevel   rise_change_level         = ChangeLevel::OFF;
        static constexpr ChangeLevel   fall_change_level         = ChangeLevel::OFF;
        static constexpr float         maximum_change_duty_cycle = 0.00F;
        static constexpr float         Kp                        = 1.0F;   // 暫定
        static constexpr float         Ki                        = 0.1F;   // 暫定
        static constexpr float         Kd                        = 0.01F;  // 暫定
        static constexpr float         pulse_period              = 1.0F / 5'000.0F;
        static constexpr float         release_time              = 0.500F;
        static constexpr Decay         decay                     = Decay::Slow;
        static constexpr PwmSide       pwm_side                  = PwmSide::Low;
        static constexpr bool          reset                     = false;
        static constexpr bool          sleep                     = false;
    };

    static constexpr float min_pulse_period = 1.0F / 60'000.0F;
    static constexpr float max_pulse_period = 1.0F / 10.0F;

    /// デューティ比の変化具合の最小値 @n
    /// Float 型の 1.00 に対してこれよりも小さい値を足しても、情報落ちして 1.00 より大きくならない
    static constexpr float minimum_maximum_change_duty_cycle = std::pow(2.0F, -23.0F);

private:
    ControlSystem _control_system{Default::control_system};
    float         _duty_cycle{0.00F};
    float         _speed{0.00F};
    float         _Kp{Default::Kp};
    float         _Ki{Default::Ki};
    float         _Kd{Default::Kd};
    State         _state{Default::state};
    ChangeLevel   _rise_change_level{Default::rise_change_level};
    ChangeLevel   _fall_change_level{Default::fall_change_level};
    float         _rise_maximum_change_duty_cycle{Default::maximum_change_duty_cycle};
    float         _fall_maximum_change_duty_cycle{Default::maximum_change_duty_cycle};
    float         _pulse_period{Default::pulse_period};
    float         _release_time{Default::release_time};
    Decay         _decay{Default::decay};
    PwmSide       _pwm_side{Default::pwm_side};
    bool          _reset{Default::reset};
    bool          _sleep{Default::sleep};
};

}  // namespace spirit

#endif  // SPIRIT_MOTOR_H
