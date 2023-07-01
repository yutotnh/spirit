#ifndef SPIRIT_MBED_PWMOUT_H
#define SPIRIT_MBED_PWMOUT_H

#include "mbed.h"
#include "spirit/include/InterfacePwmOut.h"

namespace spirit {

namespace mbed {

class PwmOut : public InterfacePwmOut {
public:
    /**
     * @brief Constructor
     * @param pin PwmOut pin to connect to
     */
    PwmOut(PinName pin);

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

private:
    ::mbed::PwmOut _pwm_out;
};

}  // namespace mbed

}  // namespace spirit

#endif  // SPIRIT_MBED_PWMOUT_H
