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
     * @param channel PwmOut channel to connect to
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
     * @param seconds PWM周期(s)
     */
    void period(float seconds) override;

private:
    uint8_t _pin;
    uint8_t _channel;
    float   _duty_cycle{0.00F};
};

}  // namespace arduino

}  // namespace spirit

#endif  // SPIRIT_ARDUINO_PWMOUT_H

#endif  // ARDUINO
