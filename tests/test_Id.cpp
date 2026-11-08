#include <gtest/gtest.h>

#include "Id.h"

namespace {

using namespace spirit;

/**
 * @brief can::get_motor_id() が成功し、期待したIDを返すことを確認する
 */
void expect_id(uint32_t expected, uint32_t motor_count, uint32_t motor, uint32_t dip_switch)
{
    uint32_t id = UINT32_MAX;
    EXPECT_TRUE(can::get_motor_id(motor_count, motor, dip_switch, id));
    EXPECT_EQ(expected, id);
}

/**
 * @brief can::get_motor_id() が失敗し、出力を変更しないことを確認する
 */
void expect_failure(uint32_t motor_count, uint32_t motor, uint32_t dip_switch)
{
    uint32_t id = 0x123;
    EXPECT_FALSE(can::get_motor_id(motor_count, motor, dip_switch, id));
    EXPECT_EQ(0x123U, id);
}

/**
 * @brief can::get_motor_id() のテスト
 */
TEST(Id, IdCanMotorTest)
{
    // 正常系のテスト

    /// @test モーターの数が1の場合
    // dip_switchに着目
    expect_id(0x200, 1, 0, 0x0);
    expect_id(0x27C, 1, 0, 0x1F);

    /// @test モーターの数が2の場合
    // motorに着目
    expect_id(0x2BE, 2, 0, 0x1F);
    expect_id(0x2FE, 2, 1, 0x1F);
    // dip_switchに着目(0xFは既にやっているので除外)
    expect_id(0x280, 2, 0, 0x0);

    /// @test モーターの数が3or4の場合
    // motorに着目
    expect_id(0x31F, 3, 0, 0x1F);
    expect_id(0x35F, 3, 2, 0x1F);
    expect_id(0x31F, 4, 0, 0x1F);
    expect_id(0x37F, 4, 3, 0x1F);
    // dip_switchに着目(0xFは既にやっているので除外)
    expect_id(0x300, 3, 0, 0x00);
    expect_id(0x300, 4, 0, 0x00);

    // 異常系のテスト

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    /// @test dip_switchの値がビット幅( dip_switch_width )を超えている場合
    expect_failure(1, 0, 0x20);

    /// @test motorの値がモーターの数を超えている場合
    expect_failure(1, 1, 0x0);
    expect_failure(2, 2, 0x0);
    expect_failure(3, 3, 0x0);
    expect_failure(4, 4, 0x0);

    /// @test 非対応の個数のモーターの場合
    expect_failure(0, 0, 0x0);
    expect_failure(5, 0, 0x0);
    expect_failure(8, 0, 0x0);

    testing::internal::GetCapturedStderr();
}

}  // namespace
