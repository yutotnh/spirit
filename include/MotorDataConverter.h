#ifndef SPIRIT_MOTOR_DATA_CONVERTER_H
#define SPIRIT_MOTOR_DATA_CONVERTER_H

#include <cstddef>

#include "Motor.h"

namespace spirit {

/**
 * @brief モータードライバ用のデータ変換クラス
 */
class MotorDataConverter {
public:
    /**
     * @brief モーターの情報をエンコードする
     * @param [in] motor モーター
     * @param [in] max_buffer_size エンコードしたデータの最大ビット幅
     * @param [out] buffer エンコードしたデータを格納するバッファ
     * @param [out] buffer_size エンコードしたデータのビット幅
     * @retval true  エンコード成功
     * @retval false エンコード失敗
     */
    bool encode(const Motor& motor, std::size_t max_buffer_size, uint8_t* buffer, std::size_t& buffer_size);

    /**
     * @brief モーターの情報をデコードする
     * @param [in] buffer デコードするデータ
     * @param [in] buffer_size デコードするデータのビット幅
     * @param [out] motor デコードした情報を格納するモーター
     * @retval true  デコード成功
     * @retval false デコード失敗
     */
    bool decode(const uint8_t* buffer, std::size_t buffer_size, Motor& motor);

private:
};

}  // namespace spirit

#endif  //  SPIRIT_MOTOR_DATA_CONVERTER_H
