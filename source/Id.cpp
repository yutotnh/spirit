#include "Id.h"

#include "include/Error.h"

namespace spirit {

namespace can {

namespace {

/**
 * @brief DIPスイッチの値がビット幅を超えていないかチェックする
 * @param dip_switch_size DIPスイッチの値
 * @retval true ビット幅を超えていない (正常)
 * @retval false ビット幅を超えている (異常)
 */
bool dip_switch_is_valid(uint32_t dip_switch, std::size_t dip_switch_size)
{
    // DIPスイッチの値がビット幅を超えていないかチェックする
    // 例えば、DIPスイッチの値が0b1111で、ビット幅が3の場合、0b1111は3bitで表現できないため、エラーとする
    return (dip_switch >> dip_switch_size) == 0;
}

/**
 * @brief モーター番号がモーターの数を超えていないかチェックする
 * @param motor モーター番号
 * @param motor_count モーターの数
 * @retval true モーターの数を超えていない (正常)
 * @retval false モーターの数を超えている (異常)
 */
bool motor_is_valid(uint32_t motor_count, uint32_t motor)
{
    // モーター番号がモーターの数を超えていないかチェックする
    // 例えば、モーターの数が2で、モーター番号が2の場合、モーター番号は0-1であるため、エラーとする
    return motor < motor_count;
}

}  // namespace

uint32_t get_motor_id(const uint32_t motor_count, const uint32_t motor, const uint32_t dip_switch)
{
    uint32_t id = 0;
    id          = motor_prefix << (can_id_size - motor_prefix_size);

    constexpr uint32_t motor_count_prefix_size = 2;

    if (motor_count == 0) {
        Error& error = Error::get_instance();
        error.warning(Error::Type::InvalidValue, 0, "Total number of motors is 0", __FILE__, __func__, __LINE__);
        return 0;
    }

    if (motor_is_valid(motor_count, motor) == false) {
        Error&         error            = Error::get_instance();
        constexpr char message_format[] = "Motor number (%d) is out of range (0-%d)";
        char           message[sizeof(message_format) + Error::max_uint32_t_length * 2];
        snprintf(message, sizeof(message), message_format, motor, motor_count - 1);
        error.warning(Error::Type::IllegalCombination, 0, message, __FILE__, __func__, __LINE__);
        return 0;
    }

    if (dip_switch_is_valid(dip_switch, dip_switch_size) == false) {
        Error&         error            = Error::get_instance();
        constexpr char message_format[] = "DIP switch value (%d) exceeds maximum bit width (%d)";
        char           message[sizeof(message_format) + Error::max_uint32_t_length * 2];
        snprintf(message, sizeof(message), message_format, dip_switch, dip_switch_size);
        error.warning(Error::Type::IllegalCombination, 0, message, __FILE__, __func__, __LINE__);
        return 0;
    }

    if ((motor_count == 0) || (4 < motor_count)) {
        Error&         error            = Error::get_instance();
        constexpr char message_format[] = "Unknown motor count type (%d)";
        char           message[sizeof(message_format) + Error::max_uint32_t_length];
        snprintf(message, sizeof(message), message_format, motor_count);
        error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
        return 0;
    }

    // motor_countの値によって、データを決定する
    // - motor_count = 1: 0b00
    // - motor_count = 2: 0b01
    // - motor_count = 3: 0b10 // 3つの場合、表現に必要なビットは2bitであるため、4つの場合と同じになる
    // - motor_count = 4: 0b10
    uint32_t type = 0;

    if (motor_count == 1) {
        type = 0;
    } else if (motor_count <= 2) {
        type = 1;
    } else if (motor_count <= 4) {
        type = 2;
    } else {
        Error&         error            = Error::get_instance();
        constexpr char message_format[] = "Unknown motor count type (%d)";
        char           message[sizeof(message_format) + Error::max_uint32_t_length];
        snprintf(message, sizeof(message), message_format, motor_count);
        error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
        return 0;
    }

    switch (type) {
        case 0:
            // 基板のモータは1つであることを示す
            id |= 0x00 << (can_id_size - motor_prefix_size - motor_count_prefix_size);

            // DIPスイッチの値をそのままIDに反映する
            id |= dip_switch << (can_id_size - motor_prefix_size - motor_count_prefix_size - dip_switch_size);

            return id;
        case 1:
            // 基板のモータは2つであることを示す
            id |= 0x01 << (can_id_size - motor_prefix_size - motor_count_prefix_size);

            // どちらのモーターかを示す(1はmotorの値が0-1であり、どのmotorかを1bitで表現できるため)
            id |= motor << (can_id_size - motor_prefix_size - motor_count_prefix_size - 1);

            // DIPスイッチの値をそのままIDに反映する
            id |= dip_switch << (can_id_size - motor_prefix_size - motor_count_prefix_size - 1 - dip_switch_size);

            return id;
        case 2:
            // 基板のモータは4つであることを示す
            id |= 0x02 << (can_id_size - motor_prefix_size - motor_count_prefix_size);

            // どちらのモーターかを示す(シフトの2はmotorの値が0-3であり、どのmotorかを2bitで表現できるため)
            id |= motor << (can_id_size - motor_prefix_size - motor_count_prefix_size - 2);

            // DIPスイッチの値をそのままIDに反映する
            id |= dip_switch << (can_id_size - motor_prefix_size - motor_count_prefix_size - 2 - dip_switch_size);

            return id;
        default:
            // 8個のモーターは未対応
            // 将来の拡張を考えて、0b11を残している
            // 例えば0b1100の場合8、0b1101の場合16をモーターに割り当てるなどする
            Error&         error            = Error::get_instance();
            constexpr char message_format[] = "Unknown motor count type (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, motor_count);
            error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            return 0;
    }
}

}  // namespace can

}  // namespace spirit
