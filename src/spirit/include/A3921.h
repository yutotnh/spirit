#ifndef SPIRIT_A3921_H
#define SPIRIT_A3921_H

#include <functional>

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

    /**
     * @brief A3921 のリセット機能を実行する
     * @param sleep 実行時にReset Pulse 時間分(最小: 0.1us, 最大3.5us)だけ待つ関数
     * @code {.cpp}
     * std::function<void(void)> sleep = []() { std::this_thread::sleep_for(std::chrono::microseconds(1)); }; // C++11
     * std::function<void(void)> sleep = []() { wait_us(1); }; // Mbed OS 6
     *
     * a3921.reset(sleep);
     * @endcode
     *
     */
    void reset(const std::function<void(void)>& sleep);

    /**
     * @brief デューティー比を設定する
     * @remark run() が実行されるまで、 duty_cycle() で設定した値はモーターに対して影響しない
     * @param value
     * 設定したいデューティー比
     * - 範囲: 0.00f <= value <= 1.00F
     */
    void duty_cycle(float value);

    /**
     * @brief 回転方向を設定する
     * @remark run() が実行されるまで、 state() で設定した値はモーターに対して影響しない
     * @param type Motor::State::Coast, Motor::State::CW, Motor::State::CCW, Motor::State::Brake のいずれかの設定したい回転方向
     */
    void state(Motor::State type);

    /**
     * @brief Decayを設定する
     * @remark run() が実行されるまで、decay() で設定した値はモーターに対して影響しない
     * @param type Decay::Slow, Decay::Fast のいずれか
     */
    void decay(Motor::Decay type);

    /**
     * @brief Slow DecayのPWM sideを設定する
     * @remark run() が実行されるまで、decay() で設定した値はモーターに対して影響しない
     * @param type PwmSide::Low, PwmSide::High のいずれか
     */
    void pwm_side(Motor::PwmSide type);

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
     */
    void pulse_period(float seconds);

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
