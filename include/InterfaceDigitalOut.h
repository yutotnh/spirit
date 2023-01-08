#ifndef SPIRIT_INTERFACE_DIGITALOUT_H
#define SPIRIT_INTERFACE_DIGITALOUT_H

#include <cstdint>

namespace spirit {

/**
 * @brief mbedのDigitalOutクラスを模した基底クラス
 */
class InterfaceDigitalOut {
public:
    /**
     * @brief 出力ピンの出力値を設定する
     * @param value 出力ピンの出力値
     *              - 0: Low
     *              - 1: High
     */
    virtual void write(uint32_t value) = 0;

    /**
     * @brief 出力ピンの出力を返す
     * @retval 0 Low
     * @retval 1 High
     */
    virtual uint32_t read();
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_DIGITALOUT_H
