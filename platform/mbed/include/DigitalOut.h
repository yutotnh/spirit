#ifndef SPIRIT_MBED_DIGITALOUT_H
#define SPIRIT_MBED_DIGITALOUT_H

#include "include/InterfaceDigitalOut.h"
#include "mbed.h"

namespace spirit {

namespace mbed {

class DigitalOut : public InterfaceDigitalOut {
public:
    /**
     * @brief Constructor
     * @param pin DigitalOut pin to connect to
     */
    DigitalOut(PinName pin);

    /**
     * @brief Destructor
     */
    ~DigitalOut();

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
    ::mbed::DigitalOut _digital_out;
};

}  // namespace mbed

}  // namespace spirit

#endif  // SPIRIT_MBED_DIGITALOUT_H
