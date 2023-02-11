#ifndef SPIRIT_ADJUST_DUTYCYCLE
#define SPIRIT_ADJUST_DUTYCYCLE

#include <cfloat>
#include <cmath>

#include "Motor.h"

namespace spirit {

/**
 * @brief モーターのデューティー比を調整する
 * @param target 調整対象のモーター
 * @param current_state 前回の回転方向
 * @param current_duty_cycle 前回のデューティー比
 * @param next_state 次回の回転方向
 * @param next_duty_cycle 次回のデューティー比
 */
void adjust_duty_cycle(const Motor& target, spirit::Motor::State current_state, float current_duty_cycle,
                       spirit::Motor::State& next_state, float& next_duty_cycle)
{
    const float rise_delta = target.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Rise);
    const float fall_delta = target.get_maximum_change_duty_cycle(Motor::ChangeLevelTarget::Fall);

    // 回転方向が異なる場合
    if (current_state != target.get_state()) {
        if (target.get_change_level(Motor::ChangeLevelTarget::Fall) == Motor::ChangeLevel::OFF) {
            next_state      = target.get_state();
            next_duty_cycle = target.get_duty_cycle();
        } else if (current_duty_cycle <= fall_delta) {
            next_state = target.get_state();
            if ((current_state == Motor::State::Coast) || (current_state == Motor::State::Brake)) {
                next_duty_cycle = rise_delta;
            } else {
                next_duty_cycle = 0.00F;
            }
        } else {
            next_state      = current_state;
            next_duty_cycle = current_duty_cycle - fall_delta;
        }

        return;
    }

    // 以降の処理は回転方向が同じ場合ということなので、次の回転方向を設定する
    next_state = target.get_state();

    // 設定値の方が現在のデューティー比より大きい場合
    if ((target.get_duty_cycle() - current_duty_cycle) >= rise_delta) {
        if (target.get_change_level(Motor::ChangeLevelTarget::Rise) == Motor::ChangeLevel::OFF) {
            next_duty_cycle = target.get_duty_cycle();
        } else {
            next_duty_cycle = current_duty_cycle + rise_delta;
        }
    }
    // 設定値の方が現在のデューティー比より小さい場合
    else if ((target.get_duty_cycle() - current_duty_cycle) <= -fall_delta) {
        if (target.get_change_level(Motor::ChangeLevelTarget::Fall) == Motor::ChangeLevel::OFF) {
            next_duty_cycle = target.get_duty_cycle();
        } else {
            next_duty_cycle = current_duty_cycle - fall_delta;
        }
    }
    // 設定値と現在のデューティー比が一緒 or ほぼ一緒の時
    else {
        next_duty_cycle = target.get_duty_cycle();
    }
}

}  // namespace spirit

#endif  //  SPIRIT_ADJUST_DUTYCYCLE
