#ifndef SPIRIT_MBED_MYDIGITALOUT_H
#define SPIRIT_MBED_MYDIGITALOUT_H

#include "InterfaceDigitalOut.h"
#include "mbed.h"

namespace spirit {

class MyDigitalOut : public InterfaceDigitalOut {
public:
    /**
     * @brief Constructor
     * @param pin MyDigitalOut pin to connect to
     */
    MyDigitalOut(PinName pin);

    /**
     * @brief Destructor
     */
    ~MyDigitalOut();

    /**
     * @brief 出力ピンの出力値を設定する
     * @param value 出力ピンの出力値
     *              - 0: Low
     *              - 1: High
     */
    void write(uint32_t value) override;

    /**
     * @brief 出力ピンの出力を返す
     * @retval 0 Low
     * @retval 1 High
     */
    uint32_t read() override;

private:
    mbed::DigitalOut _digital_out;
};

}  // namespace spirit

#endif  // SPIRIT_MBED_MYDIGITALOUT_H
