#include <gtest/gtest.h>

#include "Error.h"

namespace {

using namespace spirit;

/**
 * @brief Errorクラスのテスト
 */
TEST(Error, ErrorTest)
{
    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    spirit::Error& error = spirit::Error::get_instance();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    /// @test warning が呼ばれたら、エラー状態が Warning になる
    error.warning(spirit::Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Warning);

    /// @test error が呼ばれたら、エラー状態が Error になる
    error.error(spirit::Error::Type::UnknownValue, 0, __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Error);

    /// @test reset が呼ばれたら、エラー状態が Normal になる
    error.reset();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    /// @test 目視確認
    error.warning(spirit::Error::Type::IllegalCombination, 10, __FILE__, __func__, __LINE__, "Illegal combination1");
    error.error(spirit::Error::Type::IllegalCombination, 10, __FILE__, __func__, __LINE__, "Illegal combination2");
    error.warning(spirit::Error::Type::InvalidValue, 50, __FILE__, __func__, __LINE__, "Invalid value1");
    error.error(spirit::Error::Type::InvalidValue, 100, __FILE__, __func__, __LINE__, "Invalid value2");

    /// 異常系

    /// @test spirit::Error::Type が範囲外の時、エラー状態が Error になる
    error.reset();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);
    error.error(static_cast<spirit::Error::Type>(4), 0, __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Error);

    testing::internal::GetCapturedStderr();
}

}  // namespace
