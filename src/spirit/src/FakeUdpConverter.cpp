#include "spirit/include/FakeUdpConverter.h"

#include "spirit/include/Error.h"

namespace spirit {

bool FakeUdpConverter::encode(const uint8_t* payload, const std::size_t payload_bits, const std::size_t max_buffer_bits,
                              uint8_t* buffer, std::size_t& buffer_bits)
{
    // コネクションなどは初版のリリースには入れないため、現在はトランスポート層に相当するFakeUpdやFakeTcp(未実装)は、何も制御/処理をしない
    // 将来処理をさせるときのために、とりあえずヘッダとして1bit(0b0)だけデータに挿入する

    if (payload_bits == 0) {
        Error::get_instance().warning(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__, "Payload size is 0");
        return false;
    }

    buffer_bits = payload_bits + 1;
    if (max_buffer_bits < buffer_bits) {
        Error::get_instance().warning(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                      "Buffer size (%zu) is larger than the maximum buffer size (%zu)", buffer_bits,
                                      max_buffer_bits);
        return false;
    }

    std::size_t payload_byte_size = payload_bits / 8;
    if (payload_bits % 8 != 0) {
        payload_byte_size++;
    }

    buffer[0] = payload[0] >> 1;
    for (std::size_t i = 1; i < payload_byte_size; i++) {
        buffer[i] = ((payload[i - 1] & 1) << 7) + (payload[i] >> 1);
    }

    if (payload_bits % 8 == 0) {
        buffer[payload_byte_size] = (payload[payload_byte_size - 1] & 1) << 7;
    }

    return true;
}

bool FakeUdpConverter::decode(const uint8_t* buffer, const std::size_t buffer_bits, const std::size_t max_payload_bits,
                              uint8_t* payload, std::size_t& payload_bits)
{
    if (buffer_bits == 0) {
        Error::get_instance().warning(Error::Type::InvalidValue, 0, __FILE__, __func__, __LINE__, "Buffer size is 0");
        return false;
    }

    // ペイロードはbuffer_sizeよりも1小さくなるので、
    // 最大ペイロードサイズがbuffer_size - 1よりも大きい場合は、デコードできない
    if (max_payload_bits < buffer_bits - 1) {
        Error::get_instance().warning(Error::Type::IllegalCombination, 0, __FILE__, __func__, __LINE__,
                                      "Payload size(%zu) is larger than the maximum payload size(%zu)", buffer_bits - 1,
                                      max_payload_bits);
        return false;
    }

    std::size_t buffer_byte_size = buffer_bits / 8;
    if (buffer_bits % 8 != 0) {
        buffer_byte_size++;
    }

    for (std::size_t i = 0; i < buffer_byte_size - 1; i++) {
        payload[i] = ((buffer[i] & 0x7F) << 1) + (buffer[i + 1] >> 7);
    }

    if (buffer_bits % 8 != 1) {
        payload[buffer_byte_size - 1] = ((buffer[buffer_byte_size - 1] & 0x7F) << 1);
    }

    payload_bits = buffer_bits - 1;

    return true;
}

}  // namespace spirit
