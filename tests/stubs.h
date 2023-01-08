#ifndef SPIRIT_STUB_H
#define SPIRIT_STUB_H

#include "InterfaceDigitalOut.h"
#include "InterfacePwmOut.h"

namespace spirit {

class StubDigitalOut : public InterfaceDigitalOut {
public:
    /**
     * @brief 出力ピンの出力値を設定する
     * @param value 出力ピンの出力値
     *              - 0: Low
     *              - 1: High
     */
    void write(const uint32_t value) override
    {
        _value = value;
    }

    /**
     * @brief 出力ピンの出力を返す
     * @retval 0 Low
     * @retval 1 High
     */
    uint32_t read() override
    {
        return _value;
    }

private:
    /// 出力ピンの値
    uint32_t _value{0};
};

class StubPwmOut : public InterfacePwmOut {
public:
    /**
     * @brief 出力するデューティー比を設定する(%)
     * @param value 出力するデューティー比
     *              - 0.00F = 0%, 1.00F = 100%
     *              - 範囲: 0.00F <= value <= 1.00F
     */
    void write(const float value) override
    {
        _value = value;
    }

    /**
     * @brief 出力デューティー比を返す(%)
     * @return 出力デューティー比
     *         - 0.00F = 0%, 1.00F = 100%
     *         - 範囲: 0.00F <= value <= 1.00F
     */
    float read() override
    {
        return _value;
    }

    /**
     * @brief PWM周期を秒単位で設定する
     * @param seconds PWM周期(s)
     */
    void period(const float seconds) override
    {
        _period = seconds;
    }

    /**
     * @brief PWM周期(秒単位)を返す
     * @return PWM周期(s)
     */
    float read_period()
    {
        return _period;
    }

private:
    /// デューティー比
    float _value{0.00F};

    /// PWM周期(s)
    float _period{0.00F};
};

}  // namespace spirit

#endif  // SPIRIT_STUB_H
