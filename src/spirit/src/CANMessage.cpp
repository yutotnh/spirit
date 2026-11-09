#include "spirit/include/CANMessage.h"

#include <cstring>

namespace spirit {

CANMessage::CANMessage(const uint16_t id, const uint8_t* data, const uint8_t len)
{
    if (max_id < id) {
        _id = max_id;
    } else {
        _id = id;
    }

    if (data == nullptr) {
        _len = 0;
    } else if (max_data_length < len) {
        _len = max_data_length;
    } else {
        _len = len;
    }

    if (_len != 0) {
        memmove(_data, data, _len * sizeof(uint8_t));
    }
}

uint16_t CANMessage::get_id() const
{
    return _id;
}

std::size_t CANMessage::get_data(uint8_t* data, const std::size_t buffer_size) const
{
    if (data == nullptr) {
        return 0;
    }

    const std::size_t size = (buffer_size < _len) ? buffer_size : _len;
    if (size != 0) {
        memmove(data, _data, size * sizeof(uint8_t));
    }

    return size;
}

uint8_t CANMessage::get_length() const
{
    return _len;
}

// C++14 では、 static constexpr メンバーを参照渡しなどで使う(odr-use)と、クラス外の定義が必要になる (C++17 以降は不要)
#if __cplusplus < 201703L
constexpr uint8_t  CANMessage::max_data_length;
constexpr uint16_t CANMessage::max_id;
#endif

}  // namespace spirit
