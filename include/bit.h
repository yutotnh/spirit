#ifndef SPIRIT_BIT_H
#define SPIRIT_BIT_H

#include <cstddef>
#include <cstdint>

namespace spirit {

/**
 * @brief バッファの指定した範囲のビットを取得する
 * @param buffer バッファ
 * @param buffer_size バッファサイズ
 * @param start 開始位置
 * @param value_size 値のビット幅
 * @return 値
 */
uint32_t get_range_value(const uint8_t* buffer, std::size_t buffer_size, std::size_t start, std::size_t value_size);

/**
 * @brief バッファの指定した範囲のビットを設定する
 * @param value 値
 * @param start 開始位置
 * @param value_size 値のビット幅
 * @param buffer_size バッファサイズ
 * @param buffer バッファ
 * @return 成功したかどうか
 */
bool set_range_value(uint32_t value, std::size_t start, std::size_t value_size, std::size_t buffer_size,
                     uint8_t* buffer);

}  // namespace spirit

#endif  // SPIRIT_BIT_H
