#ifndef SPIRIT_CANMESSAGE_H
#define SPIRIT_CANMESSAGE_H

#include <cstddef>
#include <cstdint>

namespace spirit {

/**
 * @brief MbedのCanMessageクラスを模した基底クラス
 */
class CANMessage {
public:
    /**
     * @brief CANメッセージを作成する
     * @param id メッセージID (範囲: 0x000 - 0x7FF)
     * @param data メッセージデータ
     * @param len メッセージデータの長さ (範囲: 0 - 8)
     * @details メッセージIDが範囲外の場合は、0x7FFに設定される @n
     *          メッセージデータの長さが範囲外の場合は、8に設定される @n
     *          data が nullptr の場合は、長さ0のメッセージになる
     */
    CANMessage(uint16_t id, const uint8_t* data, uint8_t len);

    /**
     * @brief メッセージIDを返す
     * @return メッセージID
     */
    uint16_t get_id() const;

    /**
     * @brief メッセージデータをバッファにコピーする
     * @param [out] data メッセージデータを格納するバッファ
     * @param [in] buffer_size バッファのバイト数 (メッセージデータの長さより小さい場合は、入る分だけコピーする)
     * @return コピーしたバイト数 (data が nullptr の場合は 0)
     */
    std::size_t get_data(uint8_t* data, std::size_t buffer_size) const;

    /**
     * @brief メッセージデータの長さを返す
     * @return メッセージデータの長さ
     */
    uint8_t get_length() const;

    static constexpr uint8_t  max_data_length = 8;
    static constexpr uint16_t max_id          = 0x7FF;

private:
    uint16_t _id{0};
    uint8_t  _data[max_data_length]{0};
    uint8_t  _len{0};
};

}  // namespace spirit

#endif  // SPIRIT_CANMESSAGE_H
