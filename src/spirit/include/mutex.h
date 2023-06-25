#ifndef SPIRIT_MUTEX_H
#define SPIRIT_MUTEX_H

#ifdef __MBED__
#include "mbed.h"
#elif defined(ARDUINO_ARCH_STM32)
// ArduinoのSTM32ではmutexが存在しないため定義しない
#else
#include <mutex>
#endif

namespace spirit {

class mutex {
public:
    mutex()  = default;
    ~mutex() = default;

    void lock();

    void unlock();

private:
#ifdef __MBED__
    Mutex _mutex;
#elif defined(ARDUINO_ARCH_STM32)
// ArduinoのSTM32ではmutexが存在しないため定義しない
#else
    std::mutex _mutex;
#endif
};

}  // namespace spirit

#endif  //  SPIRIT_MUTEX_H
