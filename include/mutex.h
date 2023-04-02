#ifndef SPIRIT_MUTEX_H
#define SPIRIT_MUTEX_H

#if SPIRIT_MBED || __MBED__
#include "mbed.h"
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
#if SPIRIT_MBED || __MBED__
    Mutex _mutex;
#else
    std::mutex _mutex;
#endif
};

}  // namespace spirit

#endif  //  SPIRIT_MUTEX_H
