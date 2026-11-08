#ifdef ARDUINO

#include "spirit/platform/arduino/include/InterruptIn.h"

#include "spirit/include/Error.h"

// ESP32 では、割り込みの処理は IRAM に置く必要がある
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

namespace spirit {

namespace arduino {

namespace {

/// スロットを使っているインスタンス (使っていない場合は nullptr)
InterruptIn* instances[InterruptIn::max_instances] = {};

/// attachInterrupt() に渡す、スロットごとの関数 (インスタンスを渡せないので、スロットごとに用意する)
template <uint8_t Slot>
void IRAM_ATTR trampoline()
{
    InterruptIn::dispatch(Slot);
}

using TrampolineFunction = void (*)();

const TrampolineFunction trampolines[InterruptIn::max_instances] = {
    &trampoline<0>, &trampoline<1>, &trampoline<2>, &trampoline<3>,
    &trampoline<4>, &trampoline<5>, &trampoline<6>, &trampoline<7>,
};

}  // namespace

InterruptIn::InterruptIn(const uint8_t pin) : _pin(pin)
{
    for (uint8_t i = 0; i < max_instances; i++) {
        if (instances[i] == nullptr) {
            instances[i] = this;
            _slot        = static_cast<int8_t>(i);
            break;
        }
    }

    if (_slot < 0) {
        Error::get_instance().error(Error::Type::IllegalCombination, __FILE__, __func__, __LINE__,
                                    "No free interrupt slot (max %u instances)", static_cast<unsigned>(max_instances));
    }
}

InterruptIn::~InterruptIn()
{
    _rise_handler = nullptr;
    _fall_handler = nullptr;
    update_attachment();

    if (_slot >= 0) {
        instances[_slot] = nullptr;
    }
}

void InterruptIn::rise(InterruptHandler* handler)
{
    _rise_handler = handler;
    update_attachment();
}

void InterruptIn::fall(InterruptHandler* handler)
{
    _fall_handler = handler;
    update_attachment();
}

uint32_t InterruptIn::read()
{
    return digitalRead(_pin);
}

void IRAM_ATTR InterruptIn::dispatch(const uint8_t slot)
{
    if (max_instances <= slot) {
        return;
    }

    InterruptIn* self = instances[slot];
    if (self == nullptr) {
        return;
    }

    InterruptHandler* handler = nullptr;
    switch (self->_mode) {
        case RISING:
            handler = self->_rise_handler;
            break;
        case FALLING:
            handler = self->_fall_handler;
            break;
        case CHANGE:
            // 立ち上がりか立ち下がりかは、ピンの状態で判断する
            handler = (digitalRead(self->_pin) != 0) ? self->_rise_handler : self->_fall_handler;
            break;
        default:
            break;
    }

    if (handler != nullptr) {
        handler->on_interrupt();
    }
}

void InterruptIn::update_attachment()
{
    if (_slot < 0) {
        return;
    }

    int mode = 0;
    if ((_rise_handler != nullptr) && (_fall_handler != nullptr)) {
        mode = CHANGE;
    } else if (_rise_handler != nullptr) {
        mode = RISING;
    } else if (_fall_handler != nullptr) {
        mode = FALLING;
    }

    if (mode == _mode) {
        return;
    }

    if (mode == 0) {
        detachInterrupt(digitalPinToInterrupt(_pin));
    } else {
        attachInterrupt(digitalPinToInterrupt(_pin), trampolines[_slot], mode);
    }

    _mode = mode;
}

// C++14 では、 static constexpr メンバーを参照渡しなどで使う(odr-use)と、クラス外の定義が必要になる (C++17 以降は不要)
#if __cplusplus < 201703L
constexpr uint8_t InterruptIn::max_instances;
#endif

}  // namespace arduino

}  // namespace spirit

#endif  // ARDUINO
