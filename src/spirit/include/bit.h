#ifndef SPIRIT_BIT_H
#define SPIRIT_BIT_H

#include <cstddef>
#include <cstdint>

namespace spirit {

/**
 * @brief バッファの指定した範囲のビットの値を取得する
 * @param [in] buffer バッファ
 * @param [in] buffer_size バッファのバイト幅
 * @param [in] start 開始位置(0から始まる)
 * @param [in] value_size 値のビット幅
 * @param [out] value 値
 * @retval true 正常
 * @retval false エラー
 */
bool get_range_value(const uint8_t* buffer, std::size_t buffer_size, std::size_t start, std::size_t value_size,
                     uint32_t& value);

/**
 * @brief バッファの指定した範囲のビットを設定する
 * @param [in] value 値
 * @param [in] start 開始位置(0から始まる)
 * @param [in] value_size 値のビット幅
 * @param [in] buffer_size バッファのバイト幅
 * @param [out] buffer バッファ
 * @retval true 正常
 * @retval false エラー
 */
bool set_range_value(uint32_t value, std::size_t start, std::size_t value_size, std::size_t buffer_size,
                     uint8_t* buffer);

}  // namespace spirit

#endif  // SPIRIT_BIT_H
