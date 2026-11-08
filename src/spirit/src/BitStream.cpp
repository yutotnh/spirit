#include "spirit/include/BitStream.h"

#include <climits>
#include <cstring>

#include "spirit/include/bit.h"

namespace spirit {

namespace {

/// 拡張整数の1グループのビット数
constexpr std::size_t evi_group_bits = 4;

/// 拡張整数の1グループに入る値のビット数
constexpr std::size_t evi_value_bits = 3;

}  // namespace

BitReader::BitReader(const uint8_t* buffer, const std::size_t bytes)
    : _buffer(buffer), _bytes((buffer == nullptr) ? 0 : bytes)
{
}

bool BitReader::read(const std::size_t bits, uint32_t& value)
{
    if ((sizeof(value) * CHAR_BIT < bits) || (remaining_bits() < bits)) {
        return false;
    }

    if (bits == 0) {
        value = 0;
        return true;
    }

    // 範囲は上で検査済みなので、 get_range_value() は Error を通知しない
    uint32_t read_value = 0;
    if (!get_range_value(_buffer, _bytes, _position, bits, read_value)) {
        return false;  // LCOV_EXCL_LINE
    }

    value = read_value;
    _position += bits;
    return true;
}

bool BitReader::read_evi(uint32_t& value)
{
    const std::size_t start = _position;

    uint32_t result = 0;
    bool     first  = true;

    for (;;) {
        uint32_t group = 0;
        if (!read(evi_group_bits, group)) {
            _position = start;
            return false;
        }

        const bool     has_next = (group & 0x8U) != 0;
        const uint32_t digits   = group & 0x7U;

        // 先頭のグループが「続きがあって値が0」だと、最小の表現ではない
        if (first && has_next && (digits == 0)) {
            _position = start;
            return false;
        }
        first = false;

        // 3bit 追加すると32bitを超える
        if ((UINT32_MAX >> evi_value_bits) < result) {
            _position = start;
            return false;
        }

        result = (result << evi_value_bits) | digits;

        if (!has_next) {
            break;
        }
    }

    value = result;
    return true;
}

bool BitReader::skip(const std::size_t bits)
{
    if (remaining_bits() < bits) {
        return false;
    }

    _position += bits;
    return true;
}

std::size_t BitReader::remaining_bits() const
{
    return _bytes * CHAR_BIT - _position;
}

bool BitReader::remaining_is_zero() const
{
    for (std::size_t i = _position; i < _bytes * CHAR_BIT; i++) {
        if (((_buffer[i / CHAR_BIT] >> (CHAR_BIT - 1 - (i % CHAR_BIT))) & 0x1) != 0) {
            return false;
        }
    }

    return true;
}

BitWriter::BitWriter(uint8_t* buffer, const std::size_t bytes)
    : _buffer(buffer), _bytes((buffer == nullptr) ? 0 : bytes)
{
    if (_bytes != 0) {
        std::memset(_buffer, 0x00, _bytes);
    }
}

bool BitWriter::write(const uint32_t value, const std::size_t bits)
{
    if ((sizeof(value) * CHAR_BIT < bits) || (remaining_bits() < bits)) {
        return false;
    }

    if (bits == 0) {
        return true;
    }

    // 範囲は上で検査済みなので、 set_range_value() は Error を通知しない
    if (!set_range_value(value, _position, bits, _bytes, _buffer)) {
        return false;  // LCOV_EXCL_LINE
    }

    _position += bits;
    return true;
}

bool BitWriter::write_evi(const uint32_t value)
{
    if (remaining_bits() < evi_bits(value)) {
        return false;
    }

    // 3bit ずつに分けたときのグループ数 (0 でも1グループ)
    std::size_t groups = 1;
    while ((groups * evi_value_bits < sizeof(value) * CHAR_BIT) && ((value >> (groups * evi_value_bits)) != 0)) {
        groups++;
    }

    for (std::size_t i = 0; i < groups; i++) {
        const std::size_t shift    = (groups - 1 - i) * evi_value_bits;
        const uint32_t    digits   = (value >> shift) & 0x7U;
        const uint32_t    has_next = (i + 1 < groups) ? 0x8U : 0x0U;

        if (!write(has_next | digits, evi_group_bits)) {
            return false;  // LCOV_EXCL_LINE
        }
    }

    return true;
}

std::size_t BitWriter::bits() const
{
    return _position;
}

std::size_t BitWriter::bytes() const
{
    return (_position + CHAR_BIT - 1) / CHAR_BIT;
}

std::size_t BitWriter::remaining_bits() const
{
    return _bytes * CHAR_BIT - _position;
}

std::size_t BitWriter::evi_bits(const uint32_t value)
{
    std::size_t groups = 1;
    while ((groups * evi_value_bits < sizeof(value) * CHAR_BIT) && ((value >> (groups * evi_value_bits)) != 0)) {
        groups++;
    }

    return groups * evi_group_bits;
}

}  // namespace spirit
