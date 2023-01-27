#include "CANMessage.h"

#include <cstring>

namespace spirit {

CANMessage::CANMessage(const uint16_t id, const uint8_t* data, const uint8_t len)
{
    if (max_id < id) {
        _id = max_id;
    } else {
        _id = id;
    }

    if (max_data_length < len) {
        _len = max_data_length;
    } else {
        _len = len;
    }

    memmove(_data, data, _len * sizeof(uint8_t));
}

uint16_t CANMessage::get_id() const
{
    return _id;
}

void CANMessage::get_data(uint8_t* data) const
{
    memmove(data, _data, _len * sizeof(uint8_t));
}

uint8_t CANMessage::get_length() const
{
    return _len;
}

}  // namespace spirit
