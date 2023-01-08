#ifndef SPIRIT_MBED_MYPWMOUT_H
#define SPIRIT_MBED_MYPWMOUT_H

#include "InterfacePwmOut.h"
#include "mbed.h"

namespace spirit {

class MyPwmOut : public InterfacePwmOut {
public:
    /**
     * @brief Constructor
     * @param pin MyPwmOut pin to connect to
     */
    MyPwmOut(PinName pin);

    /**
     * @brief Destructor
     */
    ~MyPwmOut();

    /**
     * @brief 出力するデューティー比を設定する(%)
     * @param value 出力するデューティー比
     *              - 0.00F = 0%, 1.00F = 100%
     *              - 範囲: 0.00F <= value <= 1.00F
     */
    void write(const float value) override;

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
    void period(const float seconds) override;

    /**
     * @brief PWM周期(秒単位)を返す
     * @return PWM周期(s)
     */
    float read_period();

private:
    /// デューティー比
    float _value{0.00F};

    /// PWM周期(s)
    float _period{0.00F};

    mbed::PwmOut _pwm_out;
};

}  // namespace spirit

#endif  // SPIRIT_MBED_MYPWMOUT_H
