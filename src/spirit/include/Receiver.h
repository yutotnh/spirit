#ifndef SPIRIT_RECEIVER_H
#define SPIRIT_RECEIVER_H

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "Motor.h"
#include "Protocol.h"

namespace spirit {

namespace protocol {

/**
 * @brief 受信したデータを Motor に反映し、別のコンテキストから整合した Motor を取り出すためのクラス
 * @details
 * CAN の受信割り込み(書き手)で receive() を呼び、メインループ(読み手)で snapshot() を呼ぶ使い方を想定している @n
 * snapshot() は、ある時点の全ての設定がそろった(更新途中でない) Motor を返す @n
 * ロックも、アトミックな交換(CAS)も使わず、アトミックな読み書き(load/store)だけで受け渡す(Simpson の四スロット方式) @n
 * そのため、割り込みの禁止やミューテックスは要らず、 Cortex-M0/M0+ のようにアトミックな交換の命令がないマイコンでも動く @n
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
    /// 四スロット: [ペア][ペア内の番号]。書き手は、読み手が使っていないペアの、最後に書いていない方に書く
    Snapshot _slots[2][2];

    /// 各ペアで、最後に書き込んだスロットの番号
    std::atomic<uint8_t> _slot_index[2];

    /// 最後に書き込んだペアの番号
    std::atomic<uint8_t> _latest{0};

    /// 読み手が読んでいるペアの番号
    std::atomic<uint8_t> _reading{0};

    /// 書き手だけが使う、最新の状態 (受信するたびに、前の状態に重ねて反映する)
    Motor    _state;
    uint32_t _control_packets{0};

    std::atomic<uint32_t> _rejected_packets{0};
    std::atomic<uint32_t> _unknown_type_packets{0};
};

}  // namespace protocol

}  // namespace spirit

#endif  // SPIRIT_RECEIVER_H
