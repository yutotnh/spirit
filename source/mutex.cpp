#include "include/mutex.h"

namespace spirit {

void mutex::lock()
{
    _mutex.lock();
}
void mutex::unlock()
{
    _mutex.unlock();
}

}  // namespace spirit
