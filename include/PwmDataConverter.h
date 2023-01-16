#ifndef SPIRIT_PWM_DATA_CONVERTER_H
#define SPIRIT_PWM_DATA_CONVERTER_H

#include <cstddef>

#include "Motor.h"

namespace spirit {

/**
 * @brief PWMデータ変換クラス
 */
class PwmDataConverter {
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
    /**
     * @brief バッファからデューティ比を取得する
     * @param buffer  バッファ
     * @return デューティ比
     */
    float get_duty_cycle(const uint8_t* buffer);

    /**
     * @brief バッファにデューティ比を設定する
     * @param [in] duty_cycle デューティ比
     * @param [out] buffer     バッファ
     */
    void set_duty_cycle(float duty_cycle, uint8_t* buffer);

    /**
     * @brief バッファから回転方向を取得する
     * @param buffer バッファ
     * @return 回転方向
     */
    Motor::State get_state(const uint8_t* buffer);

    /**
     * @brief バッファに回転方向を設定する
     * @param [in] state  回転方向
     * @param [out] buffer バッファ
     */
    void set_state(Motor::State state, uint8_t* buffer);
};

}  // namespace spirit

#endif  // SPIRIT_PWM_DATA_CONVERTER_H
