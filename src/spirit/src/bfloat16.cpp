#include "spirit/include/bfloat16.h"

#include <cmath>
#include <limits>

namespace spirit {

float bfloat16_to_float32(const uint16_t bfloat16)
{
    const uint32_t sign_bit = (bfloat16 >> 15) & 0x1;
    const uint32_t exponent = (bfloat16 >> 7) & 0xFF;
    const uint32_t mantissa = bfloat16 & 0x7F;

    float sign = 1.0F;
    if (sign_bit == 1) {
        sign = -1.0F;
    }

    if ((exponent == 0) && (mantissa == 0)) {
        return 0.0F * sign;
    } else if (exponent == 255) {
        if (mantissa == 0) {
            return std::numeric_limits<float>::infinity() * sign;
        } else {
            return std::numeric_limits<float>::quiet_NaN() * sign;
        }
    } else {
        constexpr uint32_t exponent_bias = 127 + 7;  // 127: 指数部の0x7Fのオフセット, 7: 仮数部のビット数
        const uint32_t actual_mantissa = mantissa + 128;  // ケチ表現で省略された最上位ビットを復元

        return (float)std::ldexp(actual_mantissa, exponent - exponent_bias) * sign;
    }
}

uint16_t float32_to_bfloat16(const float value)
{
    const uint32_t sign     = std::signbit(value) ? 1 : 0;
    uint32_t       exponent = 0;
    uint32_t       mantissa = 0;

    if (std::fpclassify(value) == FP_ZERO) {
        exponent = 0;
        mantissa = 0;
    } else if (std::isinf(value)) {
        exponent = 255;
        mantissa = 0;
    } else if (std::isnan(value)) {
        exponent = 255;
        mantissa = 1;
    } else {
        int32_t     exponent_int32_t = 0;
        const float mantissa_float   = std::fabs(std::frexp(value, reinterpret_cast<int *>(&exponent_int32_t))) - 0.50F;
        exponent                     = exponent_int32_t + 126;
        mantissa                     = static_cast<uint32_t>(std::ldexp(mantissa_float, 8));
    }

    return (uint16_t)((sign << 15) | (exponent << 7) | mantissa);
}

}  // namespace spirit
