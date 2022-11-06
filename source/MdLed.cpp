#include "MdLed.h"

#include <chrono>
#include <climits>

namespace spirit {

MdLed::MdLed(interfaceLeds &leds)
    : _leds(leds),
      _state(interfaceMotor::default_state),
      _mode(default_mode),
      _interval(default_interval),
      _counter(0),
      _error(0),
      _error_section(0)
{
}

void MdLed::state(const State type)
{
    _state = type;

    if (_mode == Mode::Error) {
        return;
    }

    lock();
    _mode = Mode::Normal;
    write(static_cast<uint32_t>(type));
    unlock();
}

void MdLed::write(const uint32_t type)
{
    lock();
    _leds.write(type);
    unlock();
}

State MdLed::state() const
{
    return _state;
}

uint32_t MdLed::read() const
{
    return _leds.read();
}

void MdLed::mode(const Mode mode)
{
    if (_mode == mode) {
        return;
    }

    if ((mode < Mode::Begin) && (Mode::End < mode)) {
        return;
    }

    lock();

    _counter = 0;
    _mode    = mode;

    switch (mode) {
        case Mode::Alternate:
            alternately_blink();
            break;
        case Mode::Concurrent:
            concurrently_blink();
            break;
        default:
            break;
    }

    unlock();
}

MdLed::Mode MdLed::mode() const
{
    return _mode;
}

void MdLed::reset_error()
{
    if (_mode != Mode::Error) {
        return;
    }

    _mode = Mode::Normal;
    state(_state);
}

void MdLed::coordinate()
{
    if (++_counter >= _interval) {
        _counter = 0;
        switch (_mode) {
            case Mode::Normal:
                break;
            case Mode::Alternate:
                alternately_blink();
                break;
            case Mode::Concurrent:
                concurrently_blink();
                break;
            case Mode::Error:
                error_blink();
                break;
            default:
                break;
        }
    }
}

void MdLed::blinking_rate(const uint32_t unit)
{
    _interval = unit;
}

void MdLed::error(const uint32_t status)
{
    if ((_mode == Mode::Error) && (_error == status)) {
        return;
    }

    _mode          = Mode::Error;
    _error         = status;
    _error_section = 0;
    error_blink();
}

MdLed &MdLed::operator=(const State type)
{
    state(type);
    return *this;
}

MdLed &MdLed::operator=(const Mode type)
{
    mode(type);
    return *this;
}

MdLed &MdLed::operator=(const uint32_t value)
{
    write(value);
    return *this;
}

MdLed::operator State() const
{
    return _state;
}

MdLed::operator Mode() const
{
    return _mode;
}

MdLed::operator uint32_t() const
{
    return read();
}

void MdLed::alternately_blink()
{
    if (read() == 1) {
        write(2);
    } else {
        write(1);
    }
}

void MdLed::concurrently_blink()
{
    if (read() == 0) {
        write(3);
    } else {
        write(0);
    }
}

void MdLed::error_blink()
{
    // 点滅の流れ
    // error code = 6(0b110) の場合
    // 0. 11 ... エラー番号の出力開始
    // 1. 01 ... 最下位ビットが0 -> 01
    // 2. 00 ... LEDを全てOFF
    // 3. 10 ... 下から2つ目のビットが1 -> 10
    // 4. 00 ... LEDを全てOFF
    // 5. 10 ... 最上位ビットが1 -> 10
    // 0. 11 ... エラー番号の出力開始(再)
    // 1. 01 ... 最下位ビットが0 -> 01
    //    以降繰り返し

    if (_error_section == 0) {
        // _error のビット幅を計算する
        uint32_t type_bit_size = sizeof(_error) * CHAR_BIT;
        // uint32_t type_bit_size = 32;
        _error_bit_width = 1;
        for (auto i = type_bit_size - 1; i > 0; i--) {
            if (((_error >> i) & 1) == 1) {
                _error_bit_width = i + 1;
                break;
            }
        }
    }

    if (_error_section == 0) {
        write(3);
    } else if (_error_section % 2 == 0) {
        write(0);
    } else {
        uint32_t bit   = _error_section / 2;
        uint32_t value = ((_error >> bit) & 1) ? 2 : 1;
        write(value);
    }

    _error_section++;

    if (_error_section == _error_bit_width * 2) {
        _error_section = 0;
    }
}

void MdLed::lock()
{
    // _mutex.lock();
}

void MdLed::unlock()
{
    // _mutex.unlock();
}

}  // namespace spirit
