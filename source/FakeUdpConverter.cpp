#include "FakeUdpConverter.h"

namespace spirit {

bool FakeUdpConverter::encode(const uint8_t* payload, const std::size_t payload_size, const std::size_t max_buffer_size,
                              uint8_t* buffer, std::size_t& buffer_size)
{
    // コネクションなどは初版のリリースには入れないため、現在はトランスポート層に相当するFakeUpdやFakeTcp(未実装)は、何も制御/処理をしない
    // 将来処理をさせるときのために、とりあえずヘッダとして1bit(0b0)だけデータに挿入する

    if (payload_size == 0) {
        return false;
    }

    buffer_size = payload_size + 1;
    if (buffer_size > max_buffer_size) {
        return false;
    }

    std::size_t payload_byte_size = payload_size / 8;
    if (payload_size % 8 != 0) {
        payload_byte_size++;
    }

    buffer[0] = payload[0] >> 1;
    for (std::size_t i = 1; i < payload_byte_size; i++) {
        buffer[i] = ((payload[i - 1] & 1) << 7) + (payload[i] >> 1);
    }

    if (payload_size % 8 == 0) {
        buffer[payload_byte_size] = (payload[payload_byte_size - 1] & 1) << 7;
    }

    return true;
}

bool FakeUdpConverter::decode(const uint8_t* buffer, const std::size_t buffer_size, const std::size_t max_payload_size,
                              uint8_t* payload, std::size_t& payload_size)
{
    if (buffer_size == 0) {
        return false;
    }

    // ペイロードはbuffer_sizeよりも1小さくなるので、
    // 最大ペイロードサイズがbuffer_size - 1よりも大きい場合は、デコードできない
    if (max_payload_size < buffer_size - 1) {
        return false;
    }

    std::size_t buffer_byte_size = buffer_size / 8;
    if (buffer_size % 8 != 0) {
        buffer_byte_size++;
    }

    for (std::size_t i = 0; i < buffer_byte_size - 1; i++) {
        payload[i] = ((buffer[i] & 0x7F) << 1) + (buffer[i + 1] >> 7);
    }
    payload[buffer_byte_size - 1] = ((buffer[buffer_byte_size - 1] & 0x7F) << 1);

    payload_size = buffer_size - 1;

    return true;
}

}  // namespace spirit
