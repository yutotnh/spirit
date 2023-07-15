#ifndef SPIRIT_MBED_INTERRUPTIN_H
#define SPIRIT_MBED_INTERRUPTIN_H

#include "mbed.h"
#include "spirit/include/InterfaceInterruptIn.h"

namespace spirit {

namespace mbed {

class InterruptIn : public InterfaceInterruptIn {
public:
    /**
     * @brief Constructor
     * @param pin InterruptIn pin to connect to
     */
    InterruptIn(PinName pin);

    /**
     * @brief Destructor
     */
    ~InterruptIn();

    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func_rise 立ち上がり時に呼び出される関数
     */
    void rise(Callback<void()> func_rise) override;

    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func_fall 立ち下がり時に呼び出される関数
     */
    void fall(Callback<void()> func_fall) override;

    /**
     * @brief 入力ピンの状態を返す
     * @retval 0 Low
     * @retval 1 High
     */
    uint32_t read() override;

private:
    ::mbed::InterruptIn _interrupt_in;
};

}  // namespace mbed

}  // namespace spirit

#endif  // SPIRIT_MBED_INTERRUPTIN_H