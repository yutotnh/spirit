#include "spirit/include/Receiver.h"

namespace spirit {

namespace protocol {

Receiver::Receiver(const Motor& initial) : _state(initial)
{
    for (auto& slot : _slots) {
        slot.motor           = initial;
        slot.control_packets = 0;
    }
}

DecodeResult Receiver::receive(const uint8_t* buffer, const std::size_t bytes)
{
    const DecodeResult result = decode(buffer, bytes, _state);

    switch (result.status) {
        case DecodeStatus::Applied: {
            if (result.is_control()) {
                _control_packets++;
            }

            // 書き込み用の1枚に、最新の状態を書いてから、受け渡し用の1枚と入れ替える
            _slots[_write_index].motor           = _state;
            _slots[_write_index].control_packets = _control_packets;

            // 書き込みを終えた後に公開する (release)。以前に公開されていて、読み手が取り込んでいない1枚は、次の書き込み用になる
            const uint8_t previous =
                _middle.exchange(static_cast<uint8_t>(_write_index | new_flag), std::memory_order_acq_rel);
            _write_index = static_cast<uint8_t>(previous & ~new_flag);
            break;
        }
        case DecodeStatus::UnknownType:
            _unknown_type_packets.store(_unknown_type_packets.load(std::memory_order_relaxed) + 1,
                                        std::memory_order_relaxed);
            break;
        case DecodeStatus::Malformed:
        case DecodeStatus::InvalidValue:
            _rejected_packets.store(_rejected_packets.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
            break;
    }

    return result;
}

Receiver::Snapshot Receiver::snapshot()
{
    // 新しい状態が公開されていれば、読み込み用の1枚と入れ替えて取り込む
    if ((_middle.load(std::memory_order_relaxed) & new_flag) != 0) {
        const uint8_t published = _middle.exchange(_read_index, std::memory_order_acq_rel);
        _read_index             = static_cast<uint8_t>(published & ~new_flag);
    }

    return _slots[_read_index];
}

uint32_t Receiver::rejected_packets() const
{
    return _rejected_packets.load(std::memory_order_relaxed);
}

uint32_t Receiver::unknown_type_packets() const
{
    return _unknown_type_packets.load(std::memory_order_relaxed);
}

}  // namespace protocol

}  // namespace spirit
