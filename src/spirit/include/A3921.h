#ifndef SPIRIT_A3921_H
#define SPIRIT_A3921_H

#include <functional>

#include "InterfaceDelay.h"
#include "InterfaceDigitalOut.h"
#include "InterfacePwmOut.h"
#include "Motor.h"

namespace spirit {

/**
 * @brief Allegro MicroSystems社製 フルブリッジゲートドライバ A3921 用ドライバ @n
 * @link  https://www.allegromicro.com/-/media/files/datasheets/a3921-datasheet.pdf
 * A3921 データシート
 * @endlink
 */
class A3921 {
public:
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
     * @brief コンストラクタ
     * @param sr SR pin
     * @param pwmh PWMH pin
     * @param pwml PWML pin
     * @param phase PHASE pin
     * @param reset RESET pin
     */
    A3921(InterfaceDigitalOut& sr, InterfacePwmOut& pwmh, InterfacePwmOut& pwml, InterfacePwmOut& phase,
          InterfaceDigitalOut& reset);

    /**
     * @brief A3921 を Deep Sleep
     * @param enabled
     *      - true : スリープモードを有効にする
     *      - false: スリープモードを解除する
     */
    void sleep(bool enabled);

    /// リセットパルスの幅(us) (A3921 の仕様: 最小 0.1us、最大 3.5us)
    static constexpr uint32_t reset_pulse_us = 2;

    /**
     * @brief A3921 のリセット機能を実行する
     * @param delay リセットパルスの幅(reset_pulse_us)だけ待つために使う InterfaceDelay
     * @details RESET ピンを Low にして、 reset_pulse_us だけ待ってから、 High に戻す
     */
    void reset(InterfaceDelay& delay);

    /**
     * @brief デューティー比を設定する
     * @remark run() が実行されるまで、 duty_cycle() で設定した値はモーターに対して影響しない
     * @param value
     * 設定したいデューティー比
     * - 範囲: 0.00 <= value <= 1.00
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool duty_cycle(float value);

    /**
     * @brief 回転方向を設定する
     * @remark run() が実行されるまで、 state() で設定した値はモーターに対して影響しない
     * @param type Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake のいずれかの設定したい回転方向
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool state(Motor::State type);

    /**
     * @brief Decayを設定する
     * @remark run() が実行されるまで、decay() で設定した値はモーターに対して影響しない
     * @param type Decay::Slow, Decay::Fast のいずれか
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool decay(Motor::Decay type);

    /**
     * @brief Slow DecayのPWM sideを設定する
     * @remark run() が実行されるまで、decay() で設定した値はモーターに対して影響しない
     * @param type PwmSide::Low, PwmSide::High のいずれか
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool pwm_side(Motor::PwmSide type);

    /**
     * @brief モーターを設定したデューティー比と回転方向で動かす
     * @remark run() が実行されるまで、 duty_cycle(), state(), decay() で設定した値はモーターに対して影響しない
     */
    void run();

    /**
     * @brief pwmh, pwml, phase のパルス周期を設定する
     * @param seconds
     * 設定したいパルス周期(s)
     * - 範囲: 0.1s <= seconds <= 1.6666e-5s
     * @retval true 設定した
     * @retval false 値が不正で、設定を変更しなかった (Error::error() を通知する)
     */
    bool pulse_period(float seconds);

private:
    InterfaceDigitalOut& _sr;
    InterfacePwmOut&     _pwmh;
    InterfacePwmOut&     _pwml;
    InterfacePwmOut&     _phase;
    InterfaceDigitalOut& _reset;

    float          _duty_cycle{0.00F};
    Motor::State   _state{Motor::Default::state};
    Motor::Decay   _decay{Motor::Default::decay};
    Motor::PwmSide _pwm_side{Motor::Default::pwm_side};

    /**
     * @brief Slow decayとしてモータを動かす
     * @details
     * A3921 データシートの以下の制御を実施する
     *      - Figure 1: Slow Decay Power Bridge Current Paths
     *          - (C) Slow decay, SR active, low-side PWM
     */
    void run_slow_decay();

    /**
     * @brief Fast decayとしてモータを動かす
     * @details
     * A3921 データシートの以下の制御を実施する
     *      - Figure 2: Fast Decay Power Bridge Current Paths
     *          - (B) Fast decay, SR active, full four-quadrant control
     */
    void run_fast_decay();
};

}  // namespace spirit

#endif  // SPIRIT_A3921_H
