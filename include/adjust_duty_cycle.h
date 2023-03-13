#ifndef SPIRIT_ADJUST_DUTYCYCLE
#define SPIRIT_ADJUST_DUTYCYCLE

#include <cfloat>
#include <cmath>

#include "Motor.h"

namespace spirit {

/**
 * @brief モーターのデューティー比を調整する
 * @param [in] target_state 目標の回転方向
 * @param [in] target_duty_cycle 目標のデューティー比
 * @param [in] max_rise_delta 最大上昇デューティー比
 * @param [in] max_fall_delta 最大下降デューティー比
 * @param [in] current_state 現在の回転方向
 * @param [in] current_duty_cycle 現在のデューティー比
 * @param [out] next_state 次の回転方向
 * @param [out] next_duty_cycle 次のデューティー比
 * @note
 * Motor::State::Coast, Motor::State::Brake は必ずデューティー比を0として扱う
 */
void adjust_duty_cycle(spirit::Motor::State target_state, float target_duty_cycle, float max_rise_delta,
                       float max_fall_delta, spirit::Motor::State current_state, float current_duty_cycle,
                       spirit::Motor::State& next_state, float& next_duty_cycle)
{
    // Coast <=> Brake, Coast <=> Coast, Brake <=> Brake の場合はデューティー比を0にする
    if (((current_state == Motor::State::Coast) || (current_state == Motor::State::Brake)) &&
        ((target_state == Motor::State::Coast) || (target_state == Motor::State::Brake))) {
        next_state      = target_state;
        next_duty_cycle = 0.00F;
        return;
    }

    // 現在の回転方向がCoast or Brakeの場合は現在のデューティー比を0として扱う
    if ((current_state == Motor::State::Coast) || (current_state == Motor::State::Brake)) {
        current_duty_cycle = 0.00F;
    }

    // 現在の回転方向と目標の回転方向が異なる場合
    if (current_state != target_state) {
        if (current_duty_cycle <= max_fall_delta) {
            next_state      = target_state;
            next_duty_cycle = 0.00F;
        } else {
            next_state      = current_state;
            next_duty_cycle = current_duty_cycle - max_fall_delta;
        }

        return;
    }

    // 以降の処理は回転方向が同じ場合ということなので、次の回転方向を設定する
    next_state = target_state;

    // 設定値の方が現在のデューティー比より大きい場合
    if ((target_duty_cycle - current_duty_cycle) >= max_rise_delta) {
        next_duty_cycle = current_duty_cycle + max_rise_delta;
    }
    // 設定値の方が現在のデューティー比より小さい場合
    else if ((target_duty_cycle - current_duty_cycle) <= -max_fall_delta) {
        next_duty_cycle = current_duty_cycle - max_fall_delta;
    }
    // 設定値と現在のデューティー比が一緒 or ほぼ一緒の時
    else {
        next_duty_cycle = target_duty_cycle;
    }
}

}  // namespace spirit

#endif  //  SPIRIT_ADJUST_DUTYCYCLE
