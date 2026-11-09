#include <gtest/gtest.h>

#include <cstring>
#include <random>
#include <vector>

#include "BitStream.h"
#include "Error.h"

namespace {

using namespace spirit;

TEST(BitReader, ReadFields)
{
    const uint8_t buffer[] = {0xA5, 0x0F, 0xFF};
    BitReader     reader(buffer, sizeof(buffer));

    uint32_t value = 99;
    EXPECT_EQ(reader.remaining_bits(), 24U);
    EXPECT_TRUE(reader.read(3, value));
    EXPECT_EQ(value, 0b101U);
    EXPECT_TRUE(reader.read(0, value));
    EXPECT_EQ(value, 0U);
    EXPECT_TRUE(reader.read(9, value));
    EXPECT_EQ(value, 0b001010000U);
    EXPECT_EQ(reader.remaining_bits(), 12U);
    EXPECT_TRUE(reader.read(12, value));
    EXPECT_EQ(value, 0xFFFU);
    EXPECT_EQ(reader.remaining_bits(), 0U);
}

TEST(BitReader, ReadFailureKeepsCursor)
{
    const uint8_t buffer[] = {0xFF};
    BitReader     reader(buffer, sizeof(buffer));

    uint32_t value = 123;
    EXPECT_TRUE(reader.read(6, value));
    EXPECT_FALSE(reader.read(3, value));
    EXPECT_EQ(value, 0b111111U);  // 失敗したときは値を変更しない
    EXPECT_EQ(reader.remaining_bits(), 2U);
    EXPECT_FALSE(reader.read(33, value));
    EXPECT_TRUE(reader.read(2, value));
}

TEST(BitReader, Read32Bits)
{
    const uint8_t buffer[] = {0x12, 0x34, 0x56, 0x78, 0x9A};
    BitReader     reader(buffer, sizeof(buffer));
    uint32_t      value = 0;
    EXPECT_TRUE(reader.read(32, value));
    EXPECT_EQ(value, 0x12345678U);
    EXPECT_TRUE(reader.read(8, value));
    EXPECT_EQ(value, 0x9AU);
}

TEST(BitReader, NullAndEmptyBuffer)
{
    Error::get_instance().reset();

    BitReader null_reader(nullptr, 8);
    uint32_t  value = 0;
    EXPECT_EQ(null_reader.remaining_bits(), 0U);
    EXPECT_FALSE(null_reader.read(1, value));
    EXPECT_TRUE(null_reader.remaining_is_zero());
    EXPECT_FALSE(null_reader.skip(1));
    EXPECT_TRUE(null_reader.skip(0));

    const uint8_t buffer[1] = {0};
    BitReader     empty_reader(buffer, 0);
    EXPECT_FALSE(empty_reader.read(1, value));

    // どれも Error を通知しない
    EXPECT_EQ(Error::get_instance().get_status(), Error::Status::Normal);
}

TEST(BitReader, SkipAndRemainingIsZero)
{
    const uint8_t buffer[] = {0xF0, 0x01};
    BitReader     reader(buffer, sizeof(buffer));

    EXPECT_FALSE(reader.remaining_is_zero());
    EXPECT_FALSE(reader.skip(17));
    EXPECT_TRUE(reader.skip(4));
    EXPECT_FALSE(reader.remaining_is_zero());  // 最後の1
    EXPECT_TRUE(reader.skip(11));
    EXPECT_FALSE(reader.remaining_is_zero());
    EXPECT_TRUE(reader.skip(1));
    EXPECT_TRUE(reader.remaining_is_zero());
}

TEST(BitWriter, WriteFieldsAndZeroClear)
{
    // 使い回したバッファ(全て1)でも、前の内容が残らない
    uint8_t buffer[8];
    std::memset(buffer, 0xFF, sizeof(buffer));

    BitWriter writer(buffer, sizeof(buffer));
    EXPECT_TRUE(writer.write(0b101, 3));
    EXPECT_TRUE(writer.write(0, 0));
    EXPECT_TRUE(writer.write(0xFFFFFFFFU, 2));  // 下位2bitだけが書き込まれる
    EXPECT_EQ(writer.bits(), 5U);
    EXPECT_EQ(writer.bytes(), 1U);
    EXPECT_EQ(buffer[0], 0b10111000);
    EXPECT_EQ(buffer[1], 0x00);
    EXPECT_EQ(buffer[3], 0x00);

    EXPECT_TRUE(writer.write(0x12345678U, 32));
    EXPECT_EQ(writer.bits(), 37U);
    EXPECT_EQ(writer.bytes(), 5U);  // 37bit = 5バイト
}

TEST(BitWriter, WriteFailureWritesNothing)
{
    uint8_t   buffer[1];
    BitWriter writer(buffer, sizeof(buffer));
    EXPECT_TRUE(writer.write(0, 6));
    EXPECT_FALSE(writer.write(0xFF, 3));
    EXPECT_FALSE(writer.write(0, 33));
    EXPECT_EQ(writer.bits(), 6U);
    EXPECT_EQ(buffer[0], 0x00);
    EXPECT_TRUE(writer.write(0b11, 2));
    EXPECT_EQ(buffer[0], 0b00000011);

    BitWriter null_writer(nullptr, 4);
    EXPECT_FALSE(null_writer.write(1, 1));
    EXPECT_EQ(null_writer.remaining_bits(), 0U);
}

/// EVI の符号化(PROTOCOL.md 3.2 節の表)
TEST(Evi, Encoding)
{
    struct Case {
        uint32_t    value;
        std::string bits;
    };
    const Case cases[] = {
        {0, "0000"},
        {2, "0010"},
        {7, "0111"},
        {8,
         "1001"
         "0000"},
        {16,
         "1010"
         "0000"},
        {48,
         "1110"
         "0000"},
        {50,
         "1110"
         "0010"},
        {63,
         "1111"
         "0111"},
        {64,
         "1001"
         "1000"
         "0000"},
    };

    for (const auto& c : cases) {
        uint8_t   buffer[8];
        BitWriter writer(buffer, sizeof(buffer));
        ASSERT_TRUE(writer.write_evi(c.value)) << c.value;
        EXPECT_EQ(writer.bits(), c.bits.size()) << c.value;
        EXPECT_EQ(BitWriter::evi_bits(c.value), c.bits.size()) << c.value;

        std::string actual;
        for (std::size_t i = 0; i < writer.bits(); i++) {
            actual += ((buffer[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0';
        }
        EXPECT_EQ(actual, c.bits) << c.value;
    }
}

TEST(Evi, RoundTrip)
{
    std::vector<uint32_t> values = {0, 1, 7, 8, 63, 64, 511, 512, 4095, 65535, 0xFFFFFFFEU, 0xFFFFFFFFU};
    std::mt19937          rng(1);
    for (int i = 0; i < 1000; i++) {
        values.push_back(rng() >> (rng() % 32));
    }

    for (const uint32_t value : values) {
        uint8_t   buffer[16];
        BitWriter writer(buffer, sizeof(buffer));
        ASSERT_TRUE(writer.write_evi(value)) << value;
        const std::size_t bits = writer.bits();

        BitReader reader(buffer, sizeof(buffer));
        uint32_t  decoded = 0;
        ASSERT_TRUE(reader.read_evi(decoded)) << value;
        EXPECT_EQ(decoded, value);
        EXPECT_EQ(reader.remaining_bits(), sizeof(buffer) * 8 - bits);
    }
}

TEST(Evi, RejectsMalformed)
{
    uint32_t value = 77;

    // 非最小表現 (続きがあって値が0)
    const uint8_t non_minimal[] = {0b10000001};  // 1000 0001 = 値 1 を2グループで表している
    BitReader     r1(non_minimal, 1);
    EXPECT_FALSE(r1.read_evi(value));
    EXPECT_EQ(r1.remaining_bits(), 8U);  // カーソルは動かない

    // 途中で切れている
    const uint8_t truncated[] = {0xFF};  // 続きがあるのに終わり (1111 1111 の後ろがない)
    BitReader     r2(truncated, 1);
    EXPECT_FALSE(r2.read_evi(value));
    BitReader r3(truncated, 0);
    EXPECT_FALSE(r3.read_evi(value));

    // 33bit 以上になる
    uint8_t   overflow[16];
    BitWriter writer(overflow, sizeof(overflow));
    ASSERT_TRUE(writer.write(0b1001, 4));  // 値 1、続きあり
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(writer.write(0b1111, 4));  // 続きあり、7
    }
    ASSERT_TRUE(writer.write(0b0111, 4));  // 最後
    BitReader r4(overflow, sizeof(overflow));
    EXPECT_FALSE(r4.read_evi(value));
    EXPECT_EQ(value, 77U);  // 失敗したときは値を変更しない
}

}  // namespace
