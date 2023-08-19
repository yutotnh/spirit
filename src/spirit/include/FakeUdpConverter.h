#ifndef SPIRIT_FAKE_UDP_CONVERTER_H
#define SPIRIT_FAKE_UDP_CONVERTER_H

#include <cstddef>
#include <cstdint>

namespace spirit {

class FakeUdpConverter {
public:
    /**
     * @brief FakeUdpをエンコードする
     * @param [in] payload ペイロード
     * @param [in] payload_size ペイロードのビット幅
     * @param [in] max_buffer_size エンコードしたデータの最大ビット幅
     * @param [out] buffer エンコードしたデータを格納するバッファ
     * @param [out] buffer_size エンコードしたデータのビット幅
     * @retval true エンコード成功
     * @retval false エンコード失敗
     */
    bool encode(const uint8_t* payload, std::size_t payload_size, std::size_t max_buffer_size, uint8_t* buffer,
                std::size_t& buffer_size);

    /**
     * @brief FakeUdpをデコードする
     * @param [in] buffer デコードするデータ
     * @param [in] buffer_size デコードするデータのビット幅
     * @param [in] max_payload_size デコードしたペイロードの最大ビット幅
     * @param [out] payload デコードしたペイロードを格納するバッファ
     * @param [out] payload_size デコードしたペイロードのビット幅
     * @retval true デコード成功
     * @retval false デコード失敗
     */
    bool decode(const uint8_t* buffer, std::size_t buffer_size, std::size_t max_payload_size, uint8_t* payload,
                std::size_t& payload_size);

private:
};

}  // namespace spirit

#endif  // SPIRIT_FAKE_UDP_CONVERTER_H
