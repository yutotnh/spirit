#ifndef SPIRIT_INTERFACE_DELAY_H
#define SPIRIT_INTERFACE_DELAY_H

#include <cstdint>

namespace spirit {

/**
 * @brief 指定した時間だけ待つことを表す基底クラス
 * @details プラットフォームごとの待ち方(Mbed の wait_us、 Arduino の delayMicroseconds など)は、このクラスを継承するアダプターが吸収する
 */
class InterfaceDelay {
public:
    /**
     * @brief デストラクタ
     */
    virtual ~InterfaceDelay() = default;

    /**
     * @brief 指定した時間(us)だけ待つ
     * @param microseconds 待つ時間(us)
     */
    virtual void delay_us(uint32_t microseconds) = 0;
};

}  // namespace spirit

#endif  // SPIRIT_INTERFACE_DELAY_H
