#ifndef SPIRIT_STUB_H
#define SPIRIT_STUB_H

#include "InterfaceDigitalOut.h"
#include "InterfaceInterruptIn.h"
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

class StubInterruptIn : public InterfaceInterruptIn {
public:
    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func_rise 立ち上がり時に呼び出される関数
     */
    void rise(std::function<void(void)>& func_rise) override
    {
        _func_rise = func_rise;
    }

    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func_fall 立ち下がり時に呼び出される関数
     */
    void fall(std::function<void(void)>& func_fall) override
    {
        _func_fall = func_fall;
    }

    /**
     * @brief 立ち上がり時に行われる関数呼び出しを再現
     */
    void interrupt_rise()
    {
        _value = 1;
        _func_rise();
    }

    /**
     * @brief 立ち下がり時に行われる関数呼び出しを再現
     */
    void interrupt_fall()
    {
        _value = 0;
        _func_fall();
    }

    /**
     * @brief 入力ピンの状態を返す
     * @retval 0 Low
     * @retval 1 High
     */
    uint32_t read() override
    {
        return _value;
    }

private:
    /// 立ち上がり時に呼び出す関数
    std::function<void(void)> _func_rise;

    /// 立ち下がり時に呼び出す関数
    std::function<void(void)> _func_fall;

    /// 入力ピンの状態
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
