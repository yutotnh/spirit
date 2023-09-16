#ifndef SPIRIT_ID
#define SPIRIT_ID

#include <cstddef>
#include <cstdint>

namespace spirit {

namespace can {
/// CAN通信時のIDのビット数
static constexpr uint32_t can_id_size = 11;

/// spiritでCANでモーター制御に使う場合に使うIDの先頭の値
static constexpr uint32_t motor_prefix = 0x1;

/// spiritでCANでモーター制御に使う場合に使うIDの先頭のビット幅
static constexpr uint32_t motor_prefix_size = 2;

/// spiritでCANでモーター制御に使う場合に使うIDのDIPスイッチの最大ビット幅
static constexpr uint32_t dip_switch_width = 5;

/**
 * @brief CAN通信時のモーターIDを取得する
 * @param motor_count 基板にあるモーターの数(1-4)
 * @param motor モーター番号(0-)
 * @param dip_switch DIPスイッチの値(0x0-0x1F)
 * @return CAN通信時のモーターID(0の場合、エラー)
 */
uint32_t get_motor_id(uint32_t motor_count, uint32_t motor, uint32_t dip_switch);

}  // namespace can

}  // namespace spirit

#endif  // SPIRIT_ID
