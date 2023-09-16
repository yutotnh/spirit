#ifndef SPIRIT_MUTEX_H
#define SPIRIT_MUTEX_H

#if defined(__MBED__) && !defined(ARDUINO)
#include "mbed.h"
#elif defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_MBED)
// ArduinoのSTM32/Mbedではmutexが存在しないためインクルードしない
#else
#include <mutex>
#endif

namespace spirit {

/**
 * @brief ミューテックス
 */
class mutex {
public:
    /**
     * @brief コンストラクタ
     */
    mutex() = default;

    /**
     * @brief デストラクタ
     */
    ~mutex() = default;

    /**
     * @brief ロックする
     */
    void lock();

    /**
     * @brief ロックを解除する
     */
    void unlock();

private:
#if defined(__MBED__) && !defined(ARDUINO)
    Mutex _mutex;
#elif defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_MBED)
    // ArduinoのSTM32/Mbedではmutexが存在しないため定義しない
#else
    std::mutex _mutex;
#endif
};

}  // namespace spirit

#endif  //  SPIRIT_MUTEX_H
