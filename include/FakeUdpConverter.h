#ifndef SPIRIT_FAKE_UDP_CONVERTER_H
#define SPIRIT_FAKE_UDP_CONVERTER_H

#include <cstddef>
#include <cstdint>

namespace spirit {

class FakeUdpConverter {
public:
    /**
     * @brief FakeUdpをエンコードする
     * @param payload ペイロード
     * @param payload_size ペイロードのビット幅
     * @param max_buffer_size エンコードしたデータの最大ビット幅
     * @param buffer エンコードしたデータを格納するバッファ
     * @param buffer_size エンコードしたデータのビット幅
     * @return
     */
    bool encode(const uint8_t* payload, std::size_t payload_size, const std::size_t max_buffer_size, uint8_t* buffer,
                std::size_t& buffer_size);

    /**
     * @brief FakeUdpをデコードする
     * @param buffer デコードするデータ
     * @param buffer_size デコードするデータのビット幅
     * @param max_payload_size デコードしたペイロードの最大ビット幅
     * @param payload デコードしたペイロードを格納するバッファ
     * @param payload_size デコードしたペイロードのビット幅
     * @return
     */
    bool decode(const uint8_t* buffer, std::size_t buffer_size, std::size_t max_payload_size, uint8_t* payload,
                std::size_t& payload_size);

private:
};

}  // namespace spirit

#endif  // SPIRIT_FAKE_UDP_CONVERTER_H
