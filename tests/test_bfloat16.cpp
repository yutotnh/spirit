#include <gtest/gtest.h>

#include <cmath>

#include "bfloat16.h"

TEST(BFloat16, Bfloat16ToFloat32)
{
    auto test = [](const uint16_t bfloat16, const float value) {
        float result = spirit::bfloat16_to_float32(bfloat16);
        if (std::isnan(value)) {
            EXPECT_TRUE(std::isnan(result));
        } else if (std::isinf(value)) {
            EXPECT_TRUE(std::isinf(result));
        } else {
            // bfloat16は仮数部が7bitなので、float32に変換したときに誤差が出る
            // そのため、数値が許容範囲内かどうかをチェックする
            float allowable_error_margin = value * 0.01F;
            EXPECT_NEAR(result, value, allowable_error_margin) << "bfloat16: " << result << ", value: " << value;
        }
    };

    /// @test Zero
    test(0x0000, 0.0F);
    /// @test NaN
    test(0x7F81, std::numeric_limits<float>::quiet_NaN());
    /// @test Infinity
    test(0x7F80, std::numeric_limits<float>::infinity());
    /// @test 適当な値
    test(0x3F80, 1.0F);
    test(0x3DCC, 0.1F);
    test(0x3E4C, 0.2F);
    test(0x42F6, 123.456F);
}

TEST(BFloat16, Float32ToBFloat16)
{
    auto test = [](const float value, const uint16_t bfloat16) {
        uint16_t result = spirit::float32_to_bfloat16(value);
        EXPECT_EQ(result, bfloat16);
    };

    /// @test Zero
    test(0.0F, 0x0000);
    /// @test NaN
    test(std::numeric_limits<float>::quiet_NaN(), 0x7F81);
    /// @test Infinity
    test(std::numeric_limits<float>::infinity(), 0x7F80);
    /// @test 適当な値
    test(1.0F, 0x3F80);
    test(0.1F, 0x3DCC);
    test(0.2F, 0x3E4C);
    test(123.456F, 0x42F6);
}

TEST(BFloat16, BFloat16ToFloat32AndFloat32ToBFloat16)
{
    auto test = [](const float value) {
        uint16_t bfloat16 = spirit::float32_to_bfloat16(value);

        float result = spirit::bfloat16_to_float32(bfloat16);
        EXPECT_FLOAT_EQ(result, value);
    };

    /// @test Zero
    test(0.0F);
    /// @test NaN
    test(std::numeric_limits<float>::quiet_NaN());
    /// @test Infinity
    test(std::numeric_limits<float>::infinity());
    /// @test 適当な値
    test(1.0F);
    test(0.1F);
    test(0.2F);
    test(123.456F);
}
