#ifndef SPIRIT_MBED_INTERRUPTIN_H
#define SPIRIT_MBED_INTERRUPTIN_H

// ArduinoのMbedではCallbackが使えないので、定義しない
#if !defined(ARDUINO)

#include "mbed.h"
#include "spirit/include/InterfaceInterruptIn.h"

namespace spirit {

namespace mbed {

/**
 * @brief Mbed の InterruptIn を使った InterfaceInterruptIn
 * @details Mbed の Callback は、ヒープを使わずにメンバー関数を保持できるので、 InterruptHandler をそのまま登録する
 */
class InterruptIn : public InterfaceInterruptIn {
public:
    /**
     * @brief Constructor
     * @param pin InterruptIn pin to connect to
     */
    InterruptIn(PinName pin);

    /**
     * @brief Destructor (割り込みを解除する)
     */
    ~InterruptIn();

    void rise(InterruptHandler* handler) override;

    void fall(InterruptHandler* handler) override;

    uint32_t read() override;

private:
    ::mbed::InterruptIn _interrupt_in;
};

}  // namespace mbed

}  // namespace spirit

#endif  // !defined(ARDUINO)

#endif  // SPIRIT_MBED_INTERRUPTIN_H
