#ifndef SPIRIT_INTERFACE_INTERRUPTIN_H
#define SPIRIT_INTERFACE_INTERRUPTIN_H

#include <cstdint>

#if defined(__MBED__)
#include "mbed.h"
#else
#include <functional>
#endif

namespace spirit {

/**
 * @brief mbedのInterruptInクラスを模した基底クラス
 */
class InterfaceInterruptIn {
public:
    /**
     * @brief デストラクタ
     */
    virtual ~InterfaceInterruptIn() = default;

#if defined(__MBED__)
    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func_rise 立ち上がり時に呼び出される関数
     */
    virtual void rise(Callback<void()> func_rise) = 0;
#else
    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func_rise 立ち上がり時に呼び出される関数
     */
    virtual void rise(std::function<void(void)>& func_rise) = 0;
#endif

#if defined(__MBED__)
    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func_fall 立ち下がり時に呼び出される関数
     */
    virtual void fall(Callback<void()> func_fall) = 0;
#else
    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func_fall 立ち下がり時に呼び出される関数
     */
    virtual void fall(std::function<void(void)>& func_fall) = 0;
#endif

    /**
     * @brief 入力ピンの状態を返す
     * @retval 0 Low
     * @retval 1 High
     */
    virtual uint32_t read() = 0;
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_INTERRUPTIN_H
