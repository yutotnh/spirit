#ifndef SPIRIT_ARDUINO_DIGITALOUT_H
#define SPIRIT_ARDUINO_DIGITALOUT_H

#include "Arduino.h"
#include "spirit/include/InterfaceDigitalOut.h"

namespace spirit {

namespace arduino {

class DigitalOut : public InterfaceDigitalOut {
public:
    /**
     * @brief Constructor
     * @param pin DigitalOut pin to connect to
     */
    DigitalOut(uint8_t pin);

    /**
     * @brief Destructor
     */
    ~DigitalOut();

    /**
     * @brief 出力ピンの出力値を設定する
     * @param value 出力ピンの出力値
     *              - 0: Low
     *              - 0以外: High
     */
    void write(uint32_t value) override;

    /**
     * @brief 出力ピンの出力を返す
     * @retval 0 Low
     * @retval 1 High
     */
    uint32_t read() override;

private:
    uint8_t _pin;
};

}  // namespace arduino

}  // namespace spirit

#endif  // SPIRIT_ARDUINO_DIGITALOUT_H
