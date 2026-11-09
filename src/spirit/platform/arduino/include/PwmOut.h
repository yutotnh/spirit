#ifdef ARDUINO

#ifndef SPIRIT_ARDUINO_PWMOUT_H
#define SPIRIT_ARDUINO_PWMOUT_H

#include "Arduino.h"
#include "spirit/include/InterfacePwmOut.h"

namespace spirit {

namespace arduino {

class PwmOut : public InterfacePwmOut {
public:
    /**
     * @brief Constructor
     * @param pin PwmOut pin to connect to
     * @param channel PwmOut channel to connect to (ESP32 の Arduino core 2.x 系の LEDC でのみ使用する)
     * @details ESP32 では LEDC (ハードウェア PWM) を使い、パルス周期も設定できる。 @n
     *          それ以外のボードでは analogWrite() を使うため、パルス周期は core の既定値のまま変更できない
     */
    PwmOut(uint8_t pin, uint8_t channel);

    /**
     * @brief Destructor
     */
    ~PwmOut();

    /**
     * @brief 出力するデューティー比を設定する(%)
     * @param value 出力するデューティー比
     *              - 0.00F = 0%, 1.00F = 100%
     *              - 範囲: 0.00F <= value <= 1.00F
     */
    void write(float value) override;

    /**
     * @brief 出力デューティー比を返す(%)
     * @return 出力デューティー比
     *         - 0.00F = 0%, 1.00F = 100%
     *         - 範囲: 0.00F <= value <= 1.00F
     */
    float read() override;

    /**
     * @brief PWM周期を秒単位で設定する
     * @param seconds PWM周期(s) (0 より大きい値。範囲外の値は無視する)
     * @note ESP32 以外では周期を変更できないため、値を保持するだけで出力は変わらない
     */
    void period(float seconds) override;

    /**
     * @brief 設定したPWM周期(s)を返す
     * @return PWM周期(s)
     */
    float read_period() const;

private:
    /// 現在のデューティー比を実際のピンに出力する
    void output() const;

    uint8_t _pin;
    uint8_t _channel;
    float   _duty_cycle{0.00F};
    float   _period{1.0F / 5000.0F};
};

}  // namespace arduino

}  // namespace spirit

#endif  // SPIRIT_ARDUINO_PWMOUT_H

#endif  // ARDUINO
