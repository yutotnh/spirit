#include "bit.h"

#include <climits>

#include "include/Error.h"

namespace spirit {

uint32_t get_range_value(const uint8_t* buffer, const std::size_t buffer_size, const std::size_t start,
                         const std::size_t value_size)
{
    auto end = start + value_size - 1;
    if ((buffer_size * CHAR_BIT) < end) {
        Error&         error          = Error::get_instance();
        constexpr char message_base[] = "Range (%zu-%zu) is out of range (0-%zu)";
        char           message[sizeof(message_base) + Error::max_uint32_t_length * 3];
        snprintf(message, sizeof(message), message_base, start, end, (buffer_size * CHAR_BIT) - 1);
        error.warning(Error::Type::IllegalCombination, 0, message, __FILE__, __func__, __LINE__);
        // const std::string message = "Range (" + std::to_string(start) + "-" + std::to_string(end) +
        // ") is out of range (0-" + std::to_string((buffer_size * CHAR_BIT) - 1) + ")";
        // error.warning(Error::Type::IllegalCombination, 0, message.c_str(), __FILE__, __func__, __LINE__);
        return UINT32_MAX;
    }

    uint32_t value = 0;

    for (std::size_t i = start; i <= end; ++i) {
        const auto byte  = i / 8;  // 最上位バイトから数える(0〜)
        const auto bit   = i % 8;  // 最上位ビットから数える(0〜7)
        const auto shift = 7 - bit;

        value <<= 1;
        value |= ((buffer[byte] >> shift) & 0x01);
    }
    return value;
}

bool set_range_value(const uint32_t value, const std::size_t start, const std::size_t value_size,
                     const std::size_t buffer_size, uint8_t* buffer)
{
    auto end = start + value_size - 1;
    if ((buffer_size * CHAR_BIT) < end) {
        Error&         error          = Error::get_instance();
        constexpr char message_base[] = "Range (%zu-%zu) is out of range (0-%zu)";
        char           message[sizeof(message_base) + Error::max_uint32_t_length * 3];
        snprintf(message, sizeof(message), message_base, start, end, (buffer_size * CHAR_BIT) - 1);
        error.warning(Error::Type::IllegalCombination, 0, message, __FILE__, __func__, __LINE__);
        return false;
    }

    for (std::size_t i = start; i <= end; ++i) {
        const auto byte         = i / 8;  // 最上位バイトから数える(0〜)
        const auto bit          = i % 8;  // 最上位ビットから数える(0〜7)
        const auto buffer_shift = 7 - bit;
        const auto value_shift  = end - i;

        const auto _1bit = (value >> value_shift) & 0x01;

        if (_1bit == 1) {
            buffer[byte] |= (1 << buffer_shift);
        } else {
            buffer[byte] &= ~(1 << buffer_shift);
        }
    }

    return true;
}

}  // namespace spirit
