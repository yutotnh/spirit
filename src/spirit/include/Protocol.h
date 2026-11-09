#ifndef SPIRIT_PROTOCOL_H
#define SPIRIT_PROTOCOL_H

#include <cstddef>
#include <cstdint>

#include "Motor.h"

namespace spirit {

/**
 * @brief Controller と Peripheral の間で CAN のデータ部に載せるデータの形式
 * @details 仕様は PROTOCOL.md を参照
 */
namespace protocol {

/// メッセージの型
enum class MessageType : uint8_t {
    /// PWM制御
    Pwm,
    /// 速度制御
    Speed,
    /// 設定
    Config,
    /// 未知の型、または型を判別できなかった
    Unknown,
};

/// 受信結果
enum class DecodeStatus : uint8_t {
    /// 正常に受け取り、 Motor を更新した
    Applied,
    /// 未知の型 (Motor は変更しない。エラーではない)
    UnknownType,
    /// 形式が不正 (長さの不整合など。 Motor は変更しない)
    Malformed,
    /// 値が不正 (範囲外、予約値など。 Motor は変更しない)
    InvalidValue,
};

/// decode() の結果
struct DecodeResult {
    DecodeStatus status{DecodeStatus::Malformed};
    MessageType  type{MessageType::Unknown};
    /// 適用した (既知の) 項目の数
    uint8_t applied_items{0};
    /// 読み飛ばした (未知の) 項目の数
    uint8_t skipped_items{0};

    /**
     * @brief release_time のタイマーをリセットしてよいか
     * @retval true 正常に受け取った制御パケット (PWM/Speed)
     * @retval false それ以外 (設定パケットでは、タイマーをリセットしない)
     */
    bool is_control() const
    {
        return (status == DecodeStatus::Applied) && ((type == MessageType::Pwm) || (type == MessageType::Speed));
    }
};

/// 全設定を1周送るときの、設定の項目数
constexpr std::size_t config_item_count = 9;

/**
 * @brief 制御パケット (PWM または Speed) を作る
 * @param [in] motor 送りたい Motor (制御方式に応じて PWM または Speed のパケットになる)
 * @param [out] buffer 作ったデータを書き込むバッファ (max_bytes バイトを0クリアする)
 * @param [in] max_bytes バッファのバイト数 (3以上)
 * @param [out] bytes 作ったデータのバイト数 (CANのDLC)
 * @retval true 成功
 * @retval false バッファが足りない、または Motor の値が不正 (Error::warning() を通知する)
 */
bool encode_control(const Motor& motor, uint8_t* buffer, std::size_t max_bytes, std::size_t& bytes);

/**
 * @brief 設定パケットを作る
 * @details 全設定を決まった順序(PROTOCOL.md 6.4 節)で並べ、 index 番目から入るだけ前から詰める @n
 *          呼び出し側は、 next_index が0に戻るまで繰り返し呼び出すと、全設定を1周送れる @n
 *          CAN 2.0 (max_bytes = 8) では、どの設定でも4フレームに収まる
 * @param [in] motor 送りたい Motor
 * @param [in] index 詰め始める項目の番号 (0 - config_item_count-1)
 * @param [out] buffer 作ったデータを書き込むバッファ (max_bytes バイトを0クリアする)
 * @param [in] max_bytes バッファのバイト数
 * @param [out] bytes 作ったデータのバイト数 (CANのDLC)
 * @param [out] next_index 次に詰める項目の番号 (全て詰め終わったら0)
 * @retval true 成功
 * @retval false index が範囲外、または1項目も入らない (Error::warning() を通知する)
 */
bool encode_config(const Motor& motor, std::size_t index, uint8_t* buffer, std::size_t max_bytes, std::size_t& bytes,
                   std::size_t& next_index);

/**
 * @brief 受信したデータを解釈して、 Motor を更新する
 * @details 正常に受け取れた場合に限り、 Motor を更新する (全て適用するか、何も変更しないか) @n
 *          Error を通知しないので、 CAN の受信割り込みから呼び出してもよい @n
 *          ただし、この関数は Motor を複数の値ごと更新するので、割り込みで呼び出しながら、 @n
 *          メインループが同じ Motor を読むと、更新途中の値が見えることがある @n
 *          割り込みとメインループで受け渡す場合は、ロックなしで整合した Motor を取り出せる Receiver を使うこと
 * @param [in] buffer 受信したデータ
 * @param [in] bytes 受信したデータのバイト数
 * @param [in,out] motor 更新する Motor
 * @return 受信結果
 */
DecodeResult decode(const uint8_t* buffer, std::size_t bytes, Motor& motor);

}  // namespace protocol

}  // namespace spirit

#endif  // SPIRIT_PROTOCOL_H
