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
     * @brief 目標rpsを与えると速度制御の計算結果を返す @n
     *        積分項は誤差×dtの積算、微分項は目標値の変化で跳ね上がらないよう rps の変化に対して計算する
     * @param target_rps 速度制御の目標rps
     * @param dt         制御周期(s)
     */
    float calculation(float target_rps, float dt);

    /**
     * @brief 速度制御の計算結果にかける上限値と下限値を設定
     * @attention 初期値は上限・下限とも 0 なので、この関数を呼ぶまで calculation() は 0 を返す
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

    /**
     * @brief rpsを返す
     * @return rps
     */
    float rps();

    /**
     * @brief ロータリーエンコーダのA相が変化した際の処理を行う @n
     *        Mbed とそれ以外(テスト用のスタブ)ではコンストラクタが割り込みに登録するので、呼び出す必要はない @n
     *        Arduino の attachInterrupt() は呼び出し先にインスタンスを渡せないため、 @n
     *        A相の立ち上がり・立ち下がりの割り込みハンドラから、この関数を呼び出す
     */
    void a_phase_interrupt();

    /**
     * @brief ロータリーエンコーダのB相が変化した際の処理を行う @n
     *        A相と同様に、Arduino ではB相の割り込みハンドラから、この関数を呼び出す
     */
    void b_phase_interrupt();

private:
    InterfaceInterruptIn& _a_phase;
    InterfaceInterruptIn& _b_phase;

    float _kp{0.0f};
    float _ki{0.0f};
    float _kd{0.0f};

    static constexpr int   _ppr{200};
    static constexpr float _deg_unit{360.0f / _ppr / 4.0f};

    /// 割り込みと calculation() の両方からアクセスされる
    volatile int _angle_counter;

    bool first_loop;

    /// @note モータの最低回転速度によっては変更の余地あり(ppr=200で0.15[rps]以下が目安??)
    static constexpr int _angle_buff_max{10};
    int                  _angle_buff_index;
    int                  _angle_buff[_angle_buff_max];

    float _rps{0.00f};

    float _sum_error;
    /// 前回の rps (微分項の計算用)
    float _last_rps;

    float _high_limit{0.00f};
    float _low_limit{0.00f};

    /**
     * @brief rpsを算出して返す
     * @param dt rps
     * @return 算出されたrps
     */
    float rps_calculation(float dt);

    /**
     * @brief 元の数値にリミッターをかける
     * @param val 元の数値
     * @return リミッターをかけた数値
     */
    float limiter(float val);
};

}  // namespace spirit

#endif  // SPIRIT_SPEED_CONTROLLER_H
