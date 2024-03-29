#ifndef SPIRIT_INTERFACE_INTERRUPTIN_H
#define SPIRIT_INTERFACE_INTERRUPTIN_H

#include <cstdint>

#if defined(__MBED__) && !defined(ARDUINO)
#include "mbed.h"
#elif defined(ARDUINO)
// TODO: Arduinoの場合に必要なヘッダファイルを追加する
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

#if defined(__MBED__) && !defined(ARDUINO)
    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func 立ち上がり時に呼び出される関数
     */
    virtual void rise(Callback<void()> func) = 0;
#elif defined(ARDUINO)
    // TODO: Arduinoの場合の実装を追加する
#else
    /**
     * @brief 立ち上がり時に呼び出される関数を設定する
     * @param func_rise 立ち上がり時に呼び出される関数
     */
    virtual void rise(std::function<void(void)>& func) = 0;
#endif

#if defined(__MBED__) && !defined(ARDUINO)
    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func 立ち下がり時に呼び出される関数
     */
    virtual void fall(Callback<void()> func) = 0;
#elif defined(ARDUINO)
    // TODO: Arduinoの場合の実装を追加する
#else
    /**
     * @brief 立ち下がり時に呼び出される関数を設定する
     * @param func 立ち下がり時に呼び出される関数
     */
    virtual void fall(std::function<void(void)>& func) = 0;
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
