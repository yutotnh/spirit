#include "spirit/include/mutex.h"

namespace spirit {

void mutex::lock()
{
#if !defined(ARDUINO_ARCH_STM32)
    // ArduinoのSTM32ではmutexが存在しないため実行しない
    _mutex.lock();
#endif
}
void mutex::unlock()
{
#if !defined(ARDUINO_ARCH_STM32)
    // ArduinoのSTM32ではmutexが存在しないため実行しない
    _mutex.unlock();
#endif
}

}  // namespace spirit
