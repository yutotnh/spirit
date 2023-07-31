#include "spirit/include/bit.h"

#include <climits>
#include <cstdint>

#include "spirit/include/Error.h"

namespace spirit {

bool get_range_value(const uint8_t* buffer, const std::size_t buffer_size, const std::size_t start,
                     const std::size_t value_size, uint32_t& value)
{
    if (UINT32_WIDTH < value_size) {
        Error::get_instance().error(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                    "value_size(%zu) is greater than the bit width (%zu) of the return type (uint32_t)",
                                    value_size, UINT32_WIDTH);
        return false;
    }

    if (value_size == 0) {
        value = 0;
        return true;
    }

    // value_size > 1 なので、 buffer_size == 0 になると確実にバッファオーバーランするため、警告を出す
    if (buffer_size == 0) {
        Error::get_instance().error(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                    "buffer_size is 0");
        return false;
    }

    auto end = start + value_size - 1;
    if ((buffer_size * CHAR_BIT) - 1 < end) {
        Error::get_instance().error(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                    "Range (%zu-%zu) is out of range (0-%zu)", start, end,
                                    (buffer_size * CHAR_BIT) - 1);
        return false;
    }

    value = 0;

    for (std::size_t i = start; i <= end; ++i) {
        const auto byte  = i / 8;  // 最上位バイトから数える(0〜)
        const auto bit   = i % 8;  // 最上位ビットから数える(0〜7)
        const auto shift = 7 - bit;

        value <<= 1;
        value |= ((buffer[byte] >> shift) & 0x01);
    }

    return true;
}

bool set_range_value(const uint32_t value, const std::size_t start, const std::size_t value_size,
                     const std::size_t buffer_size, uint8_t* buffer)
{
    if (UINT32_WIDTH < value_size) {
        Error::get_instance().error(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__,
                                    "value_size(%zu) is greater than the bit width (%zu) of the return type (uint32_t)",
                                    value_size, UINT32_WIDTH);
        return false;
    }

    if (value_size == 0) {
        return true;
    }

    // value_size > 1 なので、 buffer_size == 0 になると確実にバッファオーバーランするため、警告を出す
    if (buffer_size == 0) {
        Error::get_instance().error(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                    "buffer_size is 0");
        return false;
    }

    auto end = start + value_size - 1;
    if ((buffer_size * CHAR_BIT) - 1 < end) {
        Error::get_instance().error(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                    "Range (%zu-%zu) is out of range (0-%zu)", start, end,
                                    (buffer_size * CHAR_BIT) - 1);
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
