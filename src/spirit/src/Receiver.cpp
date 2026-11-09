#include "spirit/include/Receiver.h"

namespace spirit {

namespace protocol {

Receiver::Receiver(const Motor& initial) : _state(initial)
{
    for (auto& pair : _slots) {
        for (auto& slot : pair) {
            slot.motor           = initial;
            slot.control_packets = 0;
        }
    }

    _slot_index[0].store(0);
    _slot_index[1].store(0);
}

DecodeResult Receiver::receive(const uint8_t* buffer, const std::size_t bytes)
{
    const DecodeResult result = decode(buffer, bytes, _state);

    switch (result.status) {
        case DecodeStatus::Applied: {
            if (result.is_control()) {
                _control_packets++;
            }

            // Simpson の四スロット方式 (書き手)
            // 読み手が使っていないペアを選び、そのペアで最後に書いていない方のスロットに書き込んでから、公開する
            const uint8_t pair  = static_cast<uint8_t>(_reading.load() ^ 1U);
            const uint8_t index = static_cast<uint8_t>(_slot_index[pair].load() ^ 1U);

            _slots[pair][index].motor           = _state;
            _slots[pair][index].control_packets = _control_packets;

            _slot_index[pair].store(index);
            _latest.store(pair);
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
    // Simpson の四スロット方式 (読み手)
    // 最後に書き込まれたペアを使うと宣言してから、そのペアで最後に書き込まれたスロットを読む
    const uint8_t pair = _latest.load();
    _reading.store(pair);
    const uint8_t index = _slot_index[pair].load();

    return _slots[pair][index];
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
