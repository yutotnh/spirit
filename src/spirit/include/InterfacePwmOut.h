#ifndef SPIRIT_INTERFACE_PWMOUT_H
#define SPIRIT_INTERFACE_PWMOUT_H

#include <cstdint>

namespace spirit {

/**
 * @brief mbedのPwmOutクラスを模した基底クラス
 */
class InterfacePwmOut {
public:
    /**
     * @brief デストラクタ
     */
    virtual ~InterfacePwmOut() = default;

    /**
     * @brief 出力するデューティー比を設定する(%)
     * @param value 出力するデューティー比
     *              - 0.00 = 0%, 1.00 = 100%
     *              - 範囲: 0.00 <= value <= 1.00
     */
    virtual void write(float value) = 0;

    /**
     * @brief 出力デューティー比を返す(%)
     * @return 出力デューティー比
     *         - 0.00 = 0%, 1.00 = 100%
     *         - 範囲: 0.00 <= value <= 1.00
     */
    virtual float read() = 0;

    /**
     * @brief PWM周期を秒単位で設定する
     * @param seconds PWM周期(s)
     */
    virtual void period(float seconds) = 0;
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_PWMOUT_H
