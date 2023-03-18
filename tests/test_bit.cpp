#include <gtest/gtest.h>

#include "include/Error.h"
#include "include/bit.h"

namespace {

using namespace spirit;

TEST(Bit, GetRangeValueTest)
{
    // 正常系のテスト

    /// @test value_size = 0 の場合は、0を返す
    {
        uint8_t buffer[] = {0xFF};
        EXPECT_EQ(get_range_value(buffer, 0, 0, 0), 0);
        EXPECT_EQ(get_range_value(buffer, 1, 0, 0), 0);
    }

    /// @test 1ビット取得できることの確認
    {
        // 先頭ビットのみを他のビットと変えて、明らかに取得できているか確認する
        // 1 を取得する
        uint8_t buffer[2] = {0x80, 0x00};
        EXPECT_EQ(get_range_value(buffer, 1, 0, 1), 1);

        // 0 を取得する
        buffer[0] = 0x7F;
        buffer[1] = 0xFF;
        EXPECT_EQ(get_range_value(buffer, 1, 0, 1), 0);

        // 途中のビットを取得
        buffer[0] = 0x01;
        buffer[1] = 0x00;
        EXPECT_EQ(get_range_value(buffer, 1, 7, 1), 1);

        buffer[0] = 0xEF;
        buffer[1] = 0xFF;
        EXPECT_EQ(get_range_value(buffer, 1, 3, 1), 0);
    }

    /// @test 複数ビット取得できることの確認
    {
        // 先頭ビットのみを他のビットと変えて、明らかに取得できているか確認する
        // 0b00 を取得する
        uint8_t buffer[] = {0x3F, 0xFF, 0xFF};
        EXPECT_EQ(get_range_value(buffer, 1, 0, 2), 0);

        // 0b01 を取得する
        buffer[0] = 0x40;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        EXPECT_EQ(get_range_value(buffer, 1, 0, 2), 1);

        // 0b10 を取得する
        buffer[0] = 0xBF;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        EXPECT_EQ(get_range_value(buffer, 1, 0, 2), 2);

        // 0b11 を取得する
        buffer[0] = 0xC0;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        EXPECT_EQ(get_range_value(buffer, 1, 0, 2), 3);

        // 複数ビットまたがる場合
        buffer[0] = 0x01;
        buffer[1] = 0xFF;
        buffer[2] = 0x80;
        EXPECT_EQ(get_range_value(buffer, 3, 7, 10), 0x3FF);

        buffer[0] = 0xFE;
        buffer[1] = 0x00;
        buffer[2] = 0x7F;
        EXPECT_EQ(get_range_value(buffer, 3, 7, 10), 0);

        // 全てのビットを取得
        buffer[0] = 0xAB;
        buffer[1] = 0xCD;
        buffer[2] = 0xEF;
        EXPECT_EQ(get_range_value(buffer, 3, 0, 24), 0xABCDEF);
    }

    // 異常系のテスト

    /// @test バッファのバイト幅を超える指定がされている の場合にエラー(Warning)がでることのテスト
    // get_range_value は、引数としてスタートビットとビット幅を受け取るため、bufferのバイト幅(buffer_size)と取得開始ビット番号(start)、取得ビット幅(value_size) に着目したテストを行う
    // 例えば、buffer_size = 1, start = 0, value_size = 9 の場合、bufferの0ビット目から8ビット目までを取得することになるが、bufferのバイト幅は1バイトなので、9ビット目は存在しない(warningが出る)

    auto non_normal_test = [](const std::size_t buffer_size, const std::size_t start, const std::size_t value_size) {
        Error& error = Error::get_instance();
        error.reset();
        EXPECT_EQ(error.get_status(), Error::Status::Normal);
        uint32_t value = get_range_value(nullptr, buffer_size, start, value_size);
        EXPECT_EQ(value, UINT32_MAX) << "buffer_size = " << buffer_size << ", start = " << start
                                     << ", value_size = " << value_size;
        EXPECT_EQ(error.get_status(), Error::Status::Warning)
            << "buffer_size = " << buffer_size << ", start = " << start << ", value_size = " << value_size;
    };

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    /// @test buffer_size = 0 の場合は警告が出ることの確認
    non_normal_test(0, 0, 1);

    /// @test 指定したビットの範囲がバッファを超える場合にエラー(Warning)がでることのテスト
    non_normal_test(1, 0, 9);
    non_normal_test(1, 7, 2);
    non_normal_test(1, 8, 1);
    non_normal_test(2, 0, 17);
    non_normal_test(2, 7, 10);

    testing::internal::GetCapturedStderr();
}

TEST(Bit, SetRangeValueTest)
{
    // 正常系のテスト

    /// @test value_size = 0 の場合は、必ず true が返る
    {
        uint8_t buffer[1] = {0xFF};
        EXPECT_TRUE(set_range_value(0, 0, 0, 1, buffer));
        EXPECT_EQ(buffer[0], 0xFF);

        // value_size = 0 の時は、buffer は参照されないため、nullptr でもOK
        EXPECT_TRUE(set_range_value(0, 0, 0, 0, nullptr));
    }

    /// @test 1ビット設定できることの確認
    {
        // 先頭ビットのみを他のビットと変えて、明らかに設定できているか確認する
        // 1 を設定する
        uint8_t buffer[2] = {0x00, 0x00};
        set_range_value(1, 0, 1, 1, buffer);
        EXPECT_EQ(buffer[0], 0x80);
        EXPECT_EQ(buffer[1], 0x00);

        buffer[0] = 0x80;
        buffer[1] = 0x00;
        set_range_value(1, 0, 1, 1, buffer);
        EXPECT_EQ(buffer[0], 0x80);
        EXPECT_EQ(buffer[1], 0x00);

        // 0 を設定する
        buffer[0] = 0xFF;
        buffer[1] = 0xFF;
        set_range_value(0, 0, 1, 1, buffer);
        EXPECT_EQ(buffer[0], 0x7F);
        EXPECT_EQ(buffer[1], 0xFF);

        buffer[0] = 0x7F;
        buffer[1] = 0xFF;
        set_range_value(0, 0, 1, 1, buffer);
        EXPECT_EQ(buffer[0], 0x7F);
        EXPECT_EQ(buffer[1], 0xFF);

        // 途中のビットを設定
        buffer[0] = 0xFF;
        buffer[1] = 0xFF;
        set_range_value(0, 5, 1, 1, buffer);
        EXPECT_EQ(buffer[0], 0xFB);
        EXPECT_EQ(buffer[1], 0xFF);
    }

    /// @test 複数ビット設定できることの確認
    {
        // 0b00 を設定する
        uint8_t buffer[3] = {0xFF, 0xFF, 0xFF};
        set_range_value(0, 0, 2, 3, buffer);
        EXPECT_EQ(buffer[0], 0x3F);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0xFF);

        // 既に設定されているビットを改めて設定する
        buffer[0] = 0xBF;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        set_range_value(0, 0, 2, 3, buffer);
        EXPECT_EQ(buffer[0], 0x3F);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0xFF);

        // 既に設定されているビットを改めて設定する
        buffer[0] = 0x7F;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        set_range_value(0, 0, 2, 3, buffer);
        EXPECT_EQ(buffer[0], 0x3F);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0xFF);

        // 0b01 を設定する
        buffer[0] = 0x80;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        set_range_value(1, 0, 2, 3, buffer);
        EXPECT_EQ(buffer[0], 0x40);
        EXPECT_EQ(buffer[1], 0x00);
        EXPECT_EQ(buffer[2], 0x00);

        // 0b10 を設定する
        buffer[0] = 0xFF;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        set_range_value(2, 0, 2, 2, buffer);
        EXPECT_EQ(buffer[0], 0xBF);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0xFF);

        // 0b11 を設定する
        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        set_range_value(3, 0, 2, 2, buffer);
        EXPECT_EQ(buffer[0], 0xC0);
        EXPECT_EQ(buffer[1], 0x00);
        EXPECT_EQ(buffer[2], 0x00);

        // 複数ビットまたがる場合
        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        set_range_value(0x3FF, 7, 10, 3, buffer);
        EXPECT_EQ(buffer[0], 0x01);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0x80);

        buffer[0] = 0xFF;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        set_range_value(0, 7, 10, 3, buffer);
        EXPECT_EQ(buffer[0], 0xFE);
        EXPECT_EQ(buffer[1], 0x00);
        EXPECT_EQ(buffer[2], 0x7F);

        // 全てのビットを設定
        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = 0x00;
        set_range_value(0xABCDEF, 0, 24, 3, buffer);
        EXPECT_EQ(buffer[0], 0xAB);
        EXPECT_EQ(buffer[1], 0xCD);
        EXPECT_EQ(buffer[2], 0xEF);
    }

    // 異常系のテスト

    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    auto non_normal_test = [](const std::size_t start, const std::size_t value_size, const std::size_t buffer_size) {
        Error& error = Error::get_instance();
        error.reset();
        EXPECT_EQ(error.get_status(), Error::Status::Normal);

        EXPECT_FALSE(set_range_value(0, start, value_size, buffer_size, nullptr));

        EXPECT_EQ(error.get_status(), Error::Status::Warning);
    };

    /// @test buffer_size = 0 の場合は、警告が出ることの確認
    non_normal_test(0, 1, 0);

    /// @test 指定したビットの範囲がバッファを超える場合にエラー(Warning)がでることのテスト
    non_normal_test(0, 9, 1);
    non_normal_test(7, 2, 1);
    non_normal_test(8, 1, 1);
    non_normal_test(0, 17, 2);
    non_normal_test(7, 10, 2);

    testing::internal::GetCapturedStderr();
}

}  // namespace
