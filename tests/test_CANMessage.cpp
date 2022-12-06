#include <gtest/gtest.h>

#include "CANMessage.h"

using namespace spirit;

/**
 * @brief コンストラクタのテスト
 * @details メッセージIDが0x123で、データが0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07で、長さが8のメッセージを作成する @n
 * 各データに細かく着目したテストは他で実施する
 */
TEST(CANMessage, ConstructorTest1)
{
    constexpr uint16_t id     = 0x123;
    constexpr uint8_t  data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    constexpr uint8_t  length = 8;

    CANMessage msg(id, data, length);

    EXPECT_EQ(msg.get_id(), id);
    EXPECT_EQ(msg.get_length(), length);

    uint8_t return_data[length];
    msg.get_data(return_data);
    EXPECT_EQ(memcmp(data, return_data, length), 0);
}

/**
 * @brief メッセージID の境界値テスト
 */
TEST(CANMessage, IdTest)
{
    // CANMessage の id を比較する
    auto compare_can_massage_id = [](uint16_t id, uint16_t expected_id) {
        constexpr uint8_t data[] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B};
        constexpr uint8_t length = 5;

        CANMessage msg(id, data, length);

        EXPECT_EQ(msg.get_id(), expected_id);

        // 境界値前後のデータ長が来ても、データとデータ長は変わらないことのテスト
        EXPECT_EQ(msg.get_length(), length);

        uint8_t return_data[length];
        msg.get_data(return_data);
        EXPECT_EQ(memcmp(data, return_data, length), 0);
    };

    // 型が uint16_t なので、idの最小値は0で、-1はない
    // そのため最小値 - 1 は行わない

    // 入力: 最小値
    // 期待: 最小値
    compare_can_massage_id(0, 0);

    // 入力: 最小値 + 1
    // 期待: 最小値 + 1
    compare_can_massage_id(1, 1);

    // 入力: 最大値 - 1
    // 期待: 最大値 - 1
    compare_can_massage_id(CANMessage::max_id - 1, CANMessage::max_id - 1);

    // 入力: 最大値
    // 期待: 最大値
    compare_can_massage_id(CANMessage::max_id, CANMessage::max_id);

    // 入力:   最大値 + 1
    // 期待値: 最大値
    compare_can_massage_id(CANMessage::max_id + 1, CANMessage::max_id);
}

/**
 * @brief データのテスト
 */
TEST(CANMessage, DataTest)
{
    // CANMessage の データ を比較する
    // 期待値は、入力されたデータの長さに合わせて、入力されたデータの先頭から、長さ分だけをコピーしたもの
    auto test_can_massage_data = [](uint8_t value) {
        constexpr uint16_t id     = 0x123;
        constexpr uint8_t  length = CANMessage::max_data_length;
        uint8_t            data[length]{};

        for (auto& data_value : data) {
            data_value = value;
        }

        CANMessage msg(id, data, length);

        uint8_t return_data[length];
        msg.get_data(return_data);
        EXPECT_EQ(memcmp(data, return_data, length), 0);

        // いろんな値のデータが来てもIDとデータ長は変わらないことのテスト
        EXPECT_EQ(msg.get_id(), id);
        EXPECT_EQ(msg.get_length(), length);
    };

    // 型が uint8_t なので、データの最小値は0で、-1はない
    // 同様に最大値は0xFFで、0x100はない
    // そのため最小値 - 1と最大値 + 1 は行わない

    constexpr uint8_t min_value = 0x00;
    constexpr uint8_t max_value = 0xFF;

    test_can_massage_data(min_value);
    test_can_massage_data(min_value + 1);
    test_can_massage_data(max_value - 1);
    test_can_massage_data(max_value);
}

/**
 * @brief データの長さ の境界値テスト
 */
TEST(CANMessage, LengthTest)
{
    // CANMessage の データ長 を比較する
    auto compare_can_massage_length = [](uint8_t length, uint8_t expected_length) {
        constexpr uint16_t id     = 0x123;
        constexpr uint8_t  data[] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07};

        CANMessage msg(id, data, length);

        EXPECT_EQ(msg.get_length(), expected_length);

        // 境界値前後のデータ長が来ても、IDとデータは変わらないことのテスト
        EXPECT_EQ(msg.get_id(), id);

        uint8_t return_data[CANMessage::max_data_length];
        msg.get_data(return_data);
        EXPECT_EQ(memcmp(data, return_data, expected_length), 0);
    };

    // 型が uint8_t なので、lengthの最小値は0で、-1はない
    // そのため最小値 - 1 は行わない

    // 入力: 最小値
    // 期待: 最小値
    compare_can_massage_length(0, 0);

    // 入力: 最小値 + 1
    // 期待: 最小値 + 1
    compare_can_massage_length(1, 1);

    // 入力: 最大値 - 1
    // 期待: 最大値 - 1
    compare_can_massage_length(CANMessage::max_data_length - 1, CANMessage::max_data_length - 1);

    // 入力: 最大値
    // 期待: 最大値
    compare_can_massage_length(CANMessage::max_data_length, CANMessage::max_data_length);

    // 入力:   最大値 + 1
    // 期待値: 最大値
    compare_can_massage_length(CANMessage::max_data_length + 1, CANMessage::max_data_length);
}
