#ifndef SPIRIT_INTERFACE_INTERRUPTIN_H
#define SPIRIT_INTERFACE_INTERRUPTIN_H

#include <cstdint>

namespace spirit {

/**
 * @brief 割り込みが発生したときに呼び出される処理を表す基底クラス
 * @details 割り込みコンテキストから呼び出されるので、ブロックせず、ヒープも使わないこと
 */
class InterruptHandler {
public:
    virtual ~InterruptHandler() = default;

    /**
     * @brief 割り込みが発生したときに呼び出される
     */
    virtual void on_interrupt() = 0;
};

/**
 * @brief 割り込みを検出できる入力ピンを表す基底クラス
 * @details
 * プラットフォームごとの違い(Mbed の Callback、 Arduino の関数ポインタ、 std::function など)は、 @n
 * このクラスを継承するアダプターが吸収する (ライブラリの本体は、プラットフォームに依存しない)
 */
class InterfaceInterruptIn {
public:
    /**
     * @brief デストラクタ
     */
    virtual ~InterfaceInterruptIn() = default;

    /**
     * @brief 立ち上がり時に呼び出す処理を設定する
     * @param handler 立ち上がり時に呼び出す処理 (nullptr の場合は解除する)。寿命は呼び出し側が管理する
     */
    virtual void rise(InterruptHandler* handler) = 0;

    /**
     * @brief 立ち下がり時に呼び出す処理を設定する
     * @param handler 立ち下がり時に呼び出す処理 (nullptr の場合は解除する)。寿命は呼び出し側が管理する
     */
    virtual void fall(InterruptHandler* handler) = 0;

    /**
     * @brief 入力ピンの状態を返す
     * @retval 0 Low
     * @retval 1 High
     */
    virtual uint32_t read() = 0;
};

}  // namespace spirit

#endif  //SPIRIT_INTERFACE_INTERRUPTIN_H
