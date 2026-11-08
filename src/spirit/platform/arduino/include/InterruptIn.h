#ifdef ARDUINO

#ifndef SPIRIT_ARDUINO_INTERRUPTIN_H
#define SPIRIT_ARDUINO_INTERRUPTIN_H

#include "Arduino.h"
#include "spirit/include/InterfaceInterruptIn.h"

namespace spirit {

namespace arduino {

/**
 * @brief Arduino の attachInterrupt() を使った InterfaceInterruptIn
 * @details
 * attachInterrupt() は、割り込み先にインスタンス(コンテキスト)を渡せない関数ポインタしか受け取れない @n
 * そこで、 max_instances 個の関数(トランポリン)を用意し、インスタンスごとに1つを割り当てて、 @n
 * 割り込みが発生したら、割り当てられたインスタンスの InterruptHandler を呼び出す
 * @attention
 * - 同時に使えるインスタンスは max_instances 個まで (超えるとエラーになり、割り込みは使えない)
 * - ESP32 では、割り込みの処理を IRAM に置く必要があるが、 InterruptHandler の処理(仮想関数呼び出し先)までは保証できない @n
 *   フラッシュへの書き込み中に割り込みが入ると、異常終了する可能性がある
 */
class InterruptIn : public InterfaceInterruptIn {
public:
    /// 同時に使えるインスタンスの最大数
    static constexpr uint8_t max_instances = 8;

    /**
     * @brief コンストラクタ
     * @param pin 割り込みを検出するピン
     */
    explicit InterruptIn(uint8_t pin);

    /**
     * @brief デストラクタ (割り込みを解除する)
     */
    ~InterruptIn() override;

    InterruptIn(const InterruptIn&)            = delete;
    InterruptIn& operator=(const InterruptIn&) = delete;

    void rise(InterruptHandler* handler) override;

    void fall(InterruptHandler* handler) override;

    uint32_t read() override;

    /**
     * @brief 割り込みから呼び出される (内部用。トランポリンが呼び出す)
     * @param slot 割り当てられたスロットの番号
     */
    static void dispatch(uint8_t slot);

private:
    /// 立ち上がり・立ち下がりの処理の設定に合わせて、 attachInterrupt() し直す
    void update_attachment();

    /// 割り込みを検出するピン
    uint8_t _pin;

    /// 割り当てられたスロット (割り当てられなかった場合は -1)
    int8_t _slot{-1};

    /// attachInterrupt() に渡す割り込みのモードの型 (core によって、整数のマクロか、 PinStatus 型の列挙子になる)
    using InterruptMode = decltype(CHANGE);

    /// attachInterrupt() している場合は true
    bool _attached{false};

    /// attachInterrupt() に渡している割り込みのモード (_attached が true の場合だけ有効)
    InterruptMode _mode{CHANGE};

    InterruptHandler* _rise_handler{nullptr};
    InterruptHandler* _fall_handler{nullptr};
};

}  // namespace arduino

}  // namespace spirit

#endif  // SPIRIT_ARDUINO_INTERRUPTIN_H

#endif  // ARDUINO
