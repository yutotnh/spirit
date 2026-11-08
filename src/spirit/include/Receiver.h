#ifndef SPIRIT_RECEIVER_H
#define SPIRIT_RECEIVER_H

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "Motor.h"
#include "Protocol.h"

namespace spirit {

namespace protocol {

// 以下のアトミック変数がロックなしで動作しないターゲットでは、割り込みから安全に呼び出せない
static_assert(ATOMIC_INT_LOCK_FREE == 2, "std::atomic<int> must be lock-free to be used from interrupts");

/**
 * @brief 受信したデータを Motor に反映し、別のコンテキストから整合した Motor を取り出すためのクラス
 * @details
 * CAN の受信割り込み(書き手)で receive() を呼び、メインループ(読み手)で snapshot() を呼ぶ使い方を想定している @n
 * 三重バッファでロックなしに受け渡すので、割り込みの禁止やミューテックスは要らず、 @n
 * snapshot() は、ある時点の全ての設定がそろった(更新途中でない) Motor を返す @n
 * 割り込みの中で呼び出しても Error を通知せず、ブロックもしない
 * @attention 書き手(receive())と読み手(snapshot())は、それぞれ1つのコンテキストだけに限る @n
 *            (複数の割り込みから receive() を呼ぶ、複数のスレッドから snapshot() を呼ぶことはできない)
 */
class Receiver {
public:
    /// snapshot() が返す、ある時点の状態
    struct Snapshot {
        /// 受信した設定と指令値を反映した Motor
        Motor motor;
        /// 正常に受け取った制御パケット(PWM/Speed)の個数 (release_time のタイマーのリセットに使う。 uint32_t で一周する)
        uint32_t control_packets{0};
    };

    /**
     * @brief コンストラクタ
     * @param initial 最初の Motor の状態 (何も受け取っていないときの snapshot() の値)
     */
    explicit Receiver(const Motor& initial = Motor());

    /**
     * @brief 受信したデータを解釈して、反映する (書き手のコンテキストだけで呼び出す)
     * @param buffer 受信したデータ
     * @param bytes 受信したデータのバイト数
     * @return 受信結果 (protocol::decode() と同じ)
     */
    DecodeResult receive(const uint8_t* buffer, std::size_t bytes);

    /**
     * @brief 最新の状態を返す (読み手のコンテキストだけで呼び出す)
     * @return 受信した状態。 receive() が更新の途中であっても、その途中の状態は見えない
     */
    Snapshot snapshot();

    /**
     * @brief 受信を拒否したデータ(形式が不正、値が不正)の個数 (どのコンテキストからでも呼び出せる)
     */
    uint32_t rejected_packets() const;

    /**
     * @brief 未知の型で捨てたデータの個数 (どのコンテキストからでも呼び出せる)
     */
    uint32_t unknown_type_packets() const;

private:
    /// 書き込み側が公開した、新しい状態があることを示すビット
    static constexpr uint8_t new_flag = 0x4;

    /// 三重バッファの1枚
    Snapshot _slots[3];

    /// 書き手だけが使う、最新の状態 (受信するたびに、前の状態に重ねて反映する)
    Motor    _state;
    uint32_t _control_packets{0};

    /// 書き手が次に書き込む枚数の番号
    uint8_t _write_index{0};
    /// 読み手が今読んでいる枚数の番号
    uint8_t _read_index{2};
    /// 受け渡し用の枚数の番号 (new_flag が付いていれば、読み手が取り込んでいない新しい状態)
    std::atomic<uint8_t> _middle{1};

    std::atomic<uint32_t> _rejected_packets{0};
    std::atomic<uint32_t> _unknown_type_packets{0};
};

}  // namespace protocol

}  // namespace spirit

#endif  // SPIRIT_RECEIVER_H
