#include <gtest/gtest.h>

#include "FakeUdpConverter.h"

using namespace spirit;

/**
 * @brief エンコード処理のテスト
 */
TEST(FakeUdpConverter, EncodeTest)
{
    //
    // 正常系
    //

    // payloadが1byteの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t payload_size = 8;
        uint8_t               payload[1]   = {0xFF};

        constexpr std::size_t max_buffer_size = 64;
        uint8_t               buffer[8]       = {};

        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, payload_size, max_buffer_size, buffer, buffer_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(buffer[0], 0x7F);
        EXPECT_EQ(buffer[1], 0x80);
        EXPECT_EQ(buffer_size, payload_size + 1);
    }

    // payloadが2byteの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t payload_size = 16;
        uint8_t               payload[2]   = {0xFF, 0xFF};

        constexpr std::size_t max_buffer_size = 64;
        uint8_t               buffer[8]       = {};

        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, payload_size, max_buffer_size, buffer, buffer_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(buffer[0], 0x7F);
        EXPECT_EQ(buffer[1], 0xFF);
        EXPECT_EQ(buffer[2], 0x80);
        EXPECT_EQ(buffer_size, payload_size + 1);
    }

    // payloadが7bitの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t payload_size = 7;
        uint8_t               payload[1]   = {0xFE};

        constexpr std::size_t max_buffer_size = 64;
        uint8_t               buffer[8]       = {};

        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, payload_size, max_buffer_size, buffer, buffer_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(buffer[0], 0x7F);
        EXPECT_EQ(buffer_size, payload_size + 1);
    }

    //
    // 異常系
    //

    // payload_size == 0
    {
        FakeUdpConverter fake_udp_converter;

        uint8_t     payload[1]  = {};
        uint8_t     buffer[8]   = {};
        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, 0, 1, buffer, buffer_size);

        EXPECT_FALSE(result);
    }

    // payload_size > max_buffer_size
    {
        FakeUdpConverter fake_udp_converter;

        uint8_t     payload[1]  = {};
        uint8_t     buffer[8]   = {};
        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, 2, 1, buffer, buffer_size);

        EXPECT_FALSE(result);
    }

    // payload_size == max_buffer_size
    // FakeUdpのデータを挿入するためサイズがpayload_sizeよりも1大きくなるため、
    // payload_size == max_buffer_size だとエラーにする必要がある
    {
        FakeUdpConverter fake_udp_converter;

        uint8_t     payload[1]  = {};
        uint8_t     buffer[8]   = {};
        std::size_t buffer_size = 0;

        bool result = fake_udp_converter.encode(payload, 1, 1, buffer, buffer_size);

        EXPECT_FALSE(result);
    }
}

/**
 * @brief エンコード処理のテスト
 */
TEST(FakeUdpConverter, DecodeTest)
{
    //
    // 正常系
    //

    // bufferが1byteの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t max_payload_size = 8;
        std::size_t           payload_size;
        uint8_t               payload[1] = {};

        uint8_t               buffer[1]   = {0x7F};
        constexpr std::size_t buffer_size = 8;

        bool result = fake_udp_converter.decode(buffer, buffer_size, max_payload_size, payload, payload_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(payload[0], 0xFE);
        EXPECT_EQ(payload_size, 7);
    }

    // bufferが2byteの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t max_payload_size = 16;
        std::size_t           payload_size;
        uint8_t               payload[2] = {};

        uint8_t               buffer[2]   = {0x7F, 0xFF};
        constexpr std::size_t buffer_size = 16;

        bool result = fake_udp_converter.decode(buffer, buffer_size, max_payload_size, payload, payload_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(payload[0], 0xFF);
        EXPECT_EQ(payload[1], 0xFE);
        EXPECT_EQ(payload_size, 15);
    }

    // bufferが7bitの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t max_payload_size = 16;
        std::size_t           payload_size;
        uint8_t               payload[2] = {};

        uint8_t               buffer[1]   = {0x7E};
        constexpr std::size_t buffer_size = 7;

        bool result = fake_udp_converter.decode(buffer, buffer_size, max_payload_size, payload, payload_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(payload[0], 0xFC);
        EXPECT_EQ(payload_size, 6);
    }

    // bufferが9bitの場合
    {
        FakeUdpConverter fake_udp_converter;

        constexpr std::size_t max_payload_size = 16;
        std::size_t           payload_size;
        uint8_t               payload[1] = {};

        uint8_t               buffer[2]   = {0x7F, 0x80};
        constexpr std::size_t buffer_size = 9;

        bool result = fake_udp_converter.decode(buffer, buffer_size, max_payload_size, payload, payload_size);

        EXPECT_TRUE(result);
        EXPECT_EQ(payload[0], 0xFF);
        EXPECT_EQ(payload_size, 8);
    }

    //
    // 異常系
    //

    // buffer_size - 1 < max_payload_size
    {
        FakeUdpConverter fake_udp_converter;

        std::size_t payload_size;
        uint8_t     payload[1] = {};

        uint8_t buffer[1] = {};

        bool result = fake_udp_converter.decode(buffer, 3, 1, payload, payload_size);

        EXPECT_FALSE(result);
    }

    // buffer_size = 0
    {
        FakeUdpConverter fake_udp_converter;

        std::size_t payload_size;
        uint8_t     payload[1] = {};

        uint8_t buffer[1] = {};

        bool result = fake_udp_converter.decode(buffer, 0, 8, payload, payload_size);

        EXPECT_FALSE(result);
    }
}
