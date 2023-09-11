#ifndef SPIRIT_SPEED_CONTROLLER_H
#define SPIRIT_SPEED_CONTROLLER_H

#include <stdint.h>

#include "InterfaceInterruptIn.h"

namespace spirit {

class SpeedController {
public:
    /**
     * @brief コンストラクタ
     * @param a_phase RotaryEncoder A phase pin
     * @param b_phase RotaryEncoder B phase pin
     */
    SpeedController(InterfaceInterruptIn& a_phase, InterfaceInterruptIn& b_phase);

    /**
     * @brief 目標rpsを与えると速度制御の計算結果を返す
     * @param target_rps 速度制御の目標rps
     * @param dt         制御周期(s)
     */
    float calculation(float target_rps, float dt);

    /**
     * @brief rpsを算出して返す
     * @param dt rps
     * @return 算出されたrps
     */
    float rps(float dt);

    /**
     * @brief 速度制御の計算結果にかける上限値と下限値を設定
     * @param high_limit 速度制御の計算結果の最大値
     * @param low_limit  速度制御の計算結果の最小値
     */
    void limit(float high_limit, float low_limit);

    /**
     * @brief PID制御のゲイン係数を設定
     * @param kp 比例ゲイン
     * @param ki 積分ゲイン
     * @param kd 微分ゲイン
     * @retval false ゲイン変更なし
     * @retval true  ゲイン変更あり
     */
    bool pid_gain(float kp, float ki, float kd);

    /**
     * @brief ロータリーエンコーダの計算値をリセット
     */
    void reset();

    /**
     * @brief ロータリーエンコーダの角度を返す
     */
    float angle();

private:
    InterfaceInterruptIn& _a_phase;
    InterfaceInterruptIn& _b_phase;

    float _kp{0.0f};
    float _ki{0.0f};
    float _kd{0.0f};

    static constexpr int   _ppr{200};
    static constexpr float _deg_unit{360.0f / _ppr / 4.0f};

    int _angle_counter;

    bool first_loop;

    /// @note モータの最低回転速度によっては変更の余地あり(ppr=200で0.15[rps]以下が目安??)
    static constexpr int _angle_buff_max{10};
    int                  _angle_buff_index;
    int                  _angle_buff[_angle_buff_max];

    float _sum_error;
    float _delta_error;
    float _last_error;

    float _high_limit{0.00f};
    float _low_limit{0.00f};

    /**
     * @brief 元の数値にリミッターをかける
     * @param val 元の数値
     * @return リミッターをかけた数値
     */
    float limiter(float val);

    /**
     * @brief ロータリーエンコーダのA相が変化した際の処理を行う
     */
    void a_phase_interrupt();

    /**
     * @brief ロータリーエンコーダのB相が変化した際の処理を行う
     */
    void b_phase_interrupt();
};

}  // namespace spirit

#endif  // SPIRIT_SPEED_CONTROLLER_H
