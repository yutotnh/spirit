#ifndef SPIRIT_MUTEX_H

#ifdef __MBED__
#include "mbed.h"
#else
#include <mutex>
#endif  // __MBED__

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
#else
    std::mutex _mutex;
#endif  // __MBED__
};

}  // namespace spirit

#endif  //  SPIRIT_MUTEX_H
