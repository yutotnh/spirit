#include <gtest/gtest.h>

#include "Id.h"

namespace {

using namespace spirit;

/**
 * @brief can::get_motor_id() のテスト
 */
TEST(Id, IdCanMotorTest)
{
    // 正常系のテスト

    /// @test モーターの数が1の場合
    // dip_switchに着目
    EXPECT_EQ(0x200, can::get_motor_id(1, 0, 0x0));
    EXPECT_EQ(0x27C, can::get_motor_id(1, 0, 0x1F));

    /// @test モーターの数が2の場合
    // motorに着目
    EXPECT_EQ(0x2BE, can::get_motor_id(2, 0, 0x1F));
    EXPECT_EQ(0x2FE, can::get_motor_id(2, 1, 0x1F));
    // dip_switchに着目(0xFは既にやっているので除外)
    EXPECT_EQ(0x280, can::get_motor_id(2, 0, 0x0));

    /// @test モーターの数が3or4の場合
    // motorに着目
    EXPECT_EQ(0x31F, can::get_motor_id(3, 0, 0x1F));
    EXPECT_EQ(0x35F, can::get_motor_id(3, 2, 0x1F));
    EXPECT_EQ(0x31F, can::get_motor_id(4, 0, 0x1F));
    EXPECT_EQ(0x37F, can::get_motor_id(4, 3, 0x1F));
    // dip_switchに着目(0xFは既にやっているので除外)
    EXPECT_EQ(0x300, can::get_motor_id(3, 0, 0x00));
    EXPECT_EQ(0x300, can::get_motor_id(4, 0, 0x00));

    // 異常系のテスト

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    /// @test dip_switchの値がビット幅( dip_switch_size )を超えている場合
    EXPECT_EQ(0, can::get_motor_id(1, 0, 0x20));

    /// @test motorの値がモーターの数を超えている場合
    EXPECT_EQ(0, can::get_motor_id(1, 1, 0x0));
    EXPECT_EQ(0, can::get_motor_id(2, 2, 0x0));
    EXPECT_EQ(0, can::get_motor_id(3, 3, 0x0));
    EXPECT_EQ(0, can::get_motor_id(4, 4, 0x0));

    /// @test 非対応の個数のモーターの場合
    EXPECT_EQ(0, can::get_motor_id(0, 0, 0x0));
    EXPECT_EQ(0, can::get_motor_id(5, 0, 0x0));
    EXPECT_EQ(0, can::get_motor_id(8, 0, 0x0));

    testing::internal::GetCapturedStderr();
}

}  // namespace
