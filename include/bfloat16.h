#ifndef SPIRIT_BFLOAT16_H
#define SPIRIT_BFLOAT16_H

#include <cstdint>

namespace spirit {
/**
 * @brief bfloat16をfloat32に変換する
 * @param [in] bfloat16 変換元のbfloat16
 * @return 変換結果のfloat32
*/
float bfloat16_to_float32(const uint16_t bfloat16);
/**
 * @brief float32をbfloat16に変換する
 * @param [in] value 変換元のfloat32
 * @return 変換結果のbfloat16(型はuint16_t)
*/
uint16_t float32_to_bfloat16(const float value);
}  // namespace spirit

#endif  // SPIRIT_BFLOAT16_H
