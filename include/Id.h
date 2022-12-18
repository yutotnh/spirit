#ifndef SPIRIT_ID
#define SPIRIT_ID

#include <cstddef>
#include <cstdint>

namespace spirit {

namespace can {
static constexpr uint32_t can_id_size = 11;

static constexpr uint32_t motor_prefix      = 0x1;
static constexpr uint32_t motor_prefix_size = 2;
static constexpr uint32_t dip_switch_size   = 5;

/**
 * @brief CAN通信時のモーターIDを取得する
 * @param motor_count 基板にあるモーターの数
 * @param dip_switch DIPスイッチの値(0x0-0xF)
 * @param motor モーター番号
 * @return CAN通信時のモーターID(0の場合、エラー)
 */
uint32_t get_motor_id(uint32_t motor_count, uint32_t motor, uint32_t dip_switch);

}  // namespace can

}  // namespace spirit

#endif  // SPIRIT_ID
