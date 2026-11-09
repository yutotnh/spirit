#ifndef SPIRIT_STUB_H
#define SPIRIT_STUB_H

#include <string>
#include <vector>

#include "Error.h"
#include "InterfaceDelay.h"
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
     * @brief 立ち上がり時に呼び出す処理を設定する
     * @param handler 立ち上がり時に呼び出す処理 (nullptr の場合は解除する)
     */
    void rise(InterruptHandler* handler) override
    {
        _rise_handler = handler;
    }

    /**
     * @brief 立ち下がり時に呼び出す処理を設定する
     * @param handler 立ち下がり時に呼び出す処理 (nullptr の場合は解除する)
     */
    void fall(InterruptHandler* handler) override
    {
        _fall_handler = handler;
    }

    /**
     * @brief 立ち上がり時に行われる割り込みを再現
     */
    void interrupt_rise()
    {
        _value = 1;
        if (_rise_handler != nullptr) {
            _rise_handler->on_interrupt();
        }
    }

    /**
     * @brief 立ち下がり時に行われる割り込みを再現
     */
    void interrupt_fall()
    {
        _value = 0;
        if (_fall_handler != nullptr) {
            _fall_handler->on_interrupt();
        }
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

    /// 設定されている立ち上がり時の処理
    InterruptHandler* rise_handler() const
    {
        return _rise_handler;
    }

    /// 設定されている立ち下がり時の処理
    InterruptHandler* fall_handler() const
    {
        return _fall_handler;
    }

private:
    /// 立ち上がり時に呼び出す処理
    InterruptHandler* _rise_handler{nullptr};

    /// 立ち下がり時に呼び出す処理
    InterruptHandler* _fall_handler{nullptr};

    /// 入力ピンの状態
    uint32_t _value{0};
};

/// 待った時間を記録する InterfaceDelay
class StubDelay : public InterfaceDelay {
public:
    void delay_us(const uint32_t microseconds) override
    {
        calls.push_back(microseconds);
    }

    std::vector<uint32_t> calls;
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

/// エラー(警告)の情報を記録する ErrorHandler (標準エラー出力をキャプチャせずに、エラーの内容を検査できる)
class RecordingErrorHandler : public ErrorHandler {
public:
    struct Record {
        Error::Status status;
        Error::Type   type;
        std::string   filename;
        std::string   funcname;
        uint32_t      line_number;
        std::string   message;
    };

    void handle(const Error::Info& info) override
    {
        records.push_back({info.status, info.type, info.filename, info.funcname, info.line_number, info.message});
    }

    std::vector<Record> records;
};

}  // namespace spirit

#endif  // SPIRIT_STUB_H
