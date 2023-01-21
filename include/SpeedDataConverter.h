#ifndef SPIRIT_SPEED_DATA_CONVERTER_H
#define SPIRIT_SPEED_DATA_CONVERTER_H

#include <cstddef>

#include "Motor.h"

namespace spirit {

/**
 * @brief 速度データ変換クラス
 */
class SpeedDataConverter {
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
     * @brief バッファから回転速度を取得する
     * @param buffer バッファ
     * @return 回転速度
     */

    float get_speed(const uint8_t* buffer);

    /**
     * @brief バッファに回転速度を設定する
     * @param [in] speed  回転速度
     * @param [out] buffer バッファ
     */
    void set_speed(float speed, uint8_t* buffer);

    /**
     * @brief バッファからPID制御のゲイン係数を取得する
     * @param buffer バッファ
     * @param [out] Kp 比例ゲイン係数
     * @param [out] Ki 積分ゲイン係数
     * @param [out] Kd 微分ゲイン係数
    */
    void get_pid_gain_factor(const uint8_t* buffer, float& Kp, float& Ki, float& Kd);

    /**
     * @brief バッファにPID制御のゲイン係数を設定する
     * @param [in] Kp 比例ゲイン係数
     * @param [in] Ki 積分ゲイン係数
     * @param [in] Kd 微分ゲイン係数
     * @param [out] buffer バッファ
     */
    void set_pid_gain_factor(float Kp, float Ki, float Kd, uint8_t* buffer);

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

#endif  //  SPIRIT_SPEED_DATA_CONVERTER_H
