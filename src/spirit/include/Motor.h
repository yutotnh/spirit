#ifndef SPIRIT_MOTOR_H
#define SPIRIT_MOTOR_H

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
        /// 手動で全開までにかける時間(s)を入力する
        Manual,
        /// 設定したデューティー比を直ちにモーターに出力する(0秒)
        OFF,
        /// ほんの少し余裕を持たせてモーターに出力する(全開まで 0.1秒)
        Low,
        /// 少し余裕を持たせてモーターに出力する(全開まで 0.5秒)
        Middle,
        /// 余裕を持たせてモーターに出力する(全開まで 1秒)
        High,
        /// かなり余裕を持たせてモーターに出力する(全開まで 3秒)
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
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool control_system(ControlSystem type);

    /**
     * @brief 制御方式を返す
     * @return ControlSystem::PWM, ControlSystem::Speed のいずれかの設定した制御
     */
    ControlSystem get_control_system() const;

    /**
     * @brief デューティー比を設定する
     * @param value
     * 設定したいデューティー比
     * - 0.00 = 0%, 1.00 = 100%
     * - 範囲: 0.00 <= value <= 1.00
     * - 範囲を超えた場合は、範囲の端に設定される
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool duty_cycle(float value);

    /**
     * @brief デューティー比を返す
     * @param value 設定したデューティー比
     *         - 0.00 = 0%, 1.00 = 100%
     *         - 範囲: 0.00 <= value <= 1.00
     */
    float get_duty_cycle() const;

    /**
     * @brief 速度(rps)を設定する
     * @param rps 設定したい速度(rps)
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool speed(float rps);

    /**
     * @brief 速度を返す
     * @return 設定した速度(rps)
     */
    float get_speed() const;

    /**
     * @brief PID制御のゲイン係数を設定する
     * @param kp 比例ゲイン係数
     * @param ki 積分ゲイン係数
     * @param kd 微分ゲイン係数
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool pid_gain_factor(float kp, float ki, float kd);

    /**
     * @brief PID制御のゲイン係数を返す
     * @param [out] kp 比例ゲイン係数
     * @param [out] ki 積分ゲイン係数
     * @param [out] kd 微分ゲイン係数
     */
    void get_pid_gain_factor(float& kp, float& ki, float& kd) const;

    /**
     * @brief 回転方向を設定する
     * @param type State::Coast, State::CW, State::CCW, State::Brake のいずれかの設定したい回転方向
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool state(State type);

    /**
     * @brief 回転方向を返す
     * @return 設定した回転方向
     */
    State get_state() const;

    /**
     * @brief デューティ比の変化具合を設定する ChangeLevelTarget
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @param level ChangeLevel::OFF, ChangeLevel::Low, ChangeLevel::Middle, ChangeLevel::High, ChangeLevel::Max のいずれかの設定したい変化具合
     *              (ChangeLevel::Manual は指定できない。手動で設定する場合は Motor::full_scale_time() を使う)
     * @attention この関数を実行すると、それより前に Motor::full_scale_time(ChangeLevelTarget, float) で設定した値は無効になり、
     * Motor::get_full_scale_time(ChangeLevelTarget) は 設定した ChangeLevel に対応した値を返す
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool change_level(ChangeLevelTarget target, ChangeLevel level);

    /**
     * @brief デューティ比の変化具合を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @return 設定したデューティ比の変化具合
     */
    ChangeLevel get_change_level(ChangeLevelTarget target) const;

    /**
     * @brief デューティ比が 0 から 1.00 (100%) まで変化するのにかける時間(s)を設定する
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @param seconds 0 から全開(または全開から 0)までにかける時間(s)
     *                - 範囲: 0.00 <= seconds
     *                - 0.00 の場合は、変化幅を制限しない
     * @attention この関数を実行すると、それより前に Motor::change_level(ChangeLevelTarget, ChangeLevel) で設定した値は無効になり、
     * Motor::get_change_level(ChangeLevelTarget) は ChangeLevel::Manual を返す
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool full_scale_time(ChangeLevelTarget target, float seconds);

    /**
     * @brief デューティ比が 0 から 1.00 (100%) まで変化するのにかける時間(s)を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @return 0 から全開までにかける時間(s)。ChangeLevel に対応する値は以下
     *         | ChangeLevel | 時間(s) |
     *         | :---------: | :-----: |
     *         | OFF         | 0.00 (制限なし) |
     *         | Low         | 0.10    |
     *         | Middle      | 0.50    |
     *         | High        | 1.00    |
     *         | Max         | 3.00    |
     */
    float get_full_scale_time(ChangeLevelTarget target) const;

    /**
     * @brief 制御周期あたりの最大変化デューティ比を返す
     * @param target 上昇時: ChangeLevelTarget::Rise 、下降時: ChangeLevelTarget::Fall
     * @param dt 制御周期(s) (adjust_duty_cycle() を呼び出す間隔)
     * @return 最大変化デューティ比 (範囲: 0.00 < x <= 1.00)。 @n
     *         dt / get_full_scale_time(target) で、1.00 を超える場合は 1.00 @n
     *         dt が 0 以下の場合はエラーとなり、0.00 を返す
     */
    float get_maximum_change_duty_cycle(ChangeLevelTarget target, float dt) const;

    /**
     * @brief パルス周期を設定する
     * @param seconds
     * 設定したいパルス周期(s)
     * - 範囲: 1.6666e-5s <= seconds <= 0.1s
     * - 範囲を超えるとエラーになり、設定は変更されない
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool pulse_period(float seconds);

    /**
     * @brief パルス周期を返す
     * @return 設定したパルス周期(s)
     */
    float get_pulse_period() const;

    /**
     * @brief 最後にデータが来て何秒でモータを停止するかを設定する
     * @param seconds 停止までの秒数
     *      - 0.0s より小さい場合はエラーになり、設定は変更されない
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool release_time(float seconds);

    /**
     * @brief 最後にデータが来てから停止までの秒数を返す
     * @return 設定した停止までの秒数
     */
    float get_release_time() const;

    /**
     * @brief Decayを設定する
     * @param type Decay::Slow, Decay::Fast のいずれか
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool decay(Decay type);

    /**
     * @brief Decayを返す
     * @return 設定したDecay
     */
    Decay get_decay() const;

    /**
     * @brief Slow decayのPWM sideを設定する
     * @param type PwmSide::Low, PwmSide::High のいずれか
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool pwm_side(PwmSide type);

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
        static constexpr ControlSystem control_system    = ControlSystem::PWM;
        static constexpr State         state             = State::Brake;
        static constexpr ChangeLevel   rise_change_level = ChangeLevel::OFF;
        static constexpr ChangeLevel   fall_change_level = ChangeLevel::OFF;
        static constexpr float         full_scale_time   = 0.00F;
        static constexpr float         kp                = 0.30F;
        static constexpr float         ki                = 0.80F;
        static constexpr float         kd                = 0.20F;
        static constexpr float         pulse_period      = 1.0F / 5000.0F;
        static constexpr float         release_time      = 0.500F;
        static constexpr Decay         decay             = Decay::Slow;
        static constexpr PwmSide       pwm_side          = PwmSide::Low;
        static constexpr bool          reset             = false;
        static constexpr bool          sleep             = false;
    };

    static constexpr float min_pulse_period = 1.0F / 60000.0F;
    static constexpr float max_pulse_period = 1.0F / 10.0F;

private:
    ControlSystem _control_system{Default::control_system};
    float         _duty_cycle{0.00F};
    float         _speed{0.00F};
    float         _kp{Default::kp};
    float         _ki{Default::ki};
    float         _kd{Default::kd};
    State         _state{Default::state};
    ChangeLevel   _rise_change_level{Default::rise_change_level};
    ChangeLevel   _fall_change_level{Default::fall_change_level};
    float         _rise_full_scale_time{Default::full_scale_time};
    float         _fall_full_scale_time{Default::full_scale_time};
    float         _pulse_period{Default::pulse_period};
    float         _release_time{Default::release_time};
    Decay         _decay{Default::decay};
    PwmSide       _pwm_side{Default::pwm_side};
    bool          _reset{Default::reset};
    bool          _sleep{Default::sleep};
};

}  // namespace spirit

#endif  // SPIRIT_MOTOR_H
