#ifndef SPIRIT_MD_LED_H
#define SPIRIT_MD_LED_H

#include "interfaceLeds.h"
#include "interfaceMotor.h"

namespace spirit {

/**
 * @brief モーターの状態をLEDに表示するクラス
 * @details
 * モーターの駆動状態をそのまま表示するNormalモードに加え、Errorの値を表示するErrorモードなどがある
 */
class MdLed {
public:
    /**
     * @enum MdLed
     * @brief LEDに設定する値のソースを何にするかの値
     */
    enum class Mode {
        //! このクラスの範囲を取得するためのメンバ
        Begin = 0,
        //! Stateの値をそのままLEDに設定する
        Normal = Begin,
        //! LED 2つ交互に点滅   0b01 -> 0b10 -> 0b01
        Alternate,
        //! LED 2つが同じ点滅 0b11 -> 0b00 -> 0b11
        Concurrent,
        //! エラー
        Error,
        //! このクラスの範囲を取得するためのメンバ
        End = Error,
    };

    /**
     * @brief コンストラクタ
     */
    MdLed(interfaceLeds &leds);

    /**
     * @brief Normalモード時のLEDの点灯を設定する
     * @param type 設定したい値
     * @details Error中は設定値が反映されない
     */
    void state(const State type);

    /**
     * @brief LEDに設定中の値を返す
     * @return 設定中の値
     */
    State state() const;

    /**
     * @brief LEDの点滅を設定する
     * @param type 2つのLEDを2bitに置き換えた場合の値
     */
    void write(const uint32_t type);

    /**
     * @brief LEDの点滅状態を返す
     * @return LEDの点滅状態
     */
    uint32_t read() const;

    /**
     * @brief LEDのモードを設定する
     * @param type 設定したいLEDのモード
     */
    void mode(const Mode type);

    /**
     * @brief LEDのモードを返す
     * @return LEDのモード
     */
    Mode mode() const;

    /**
     * @brief LEDをError状態にする
     * @param status Error番号
     */
    void error(const uint32_t status);

    /**
     * @brief LEDのErrorモードを解除する
     */
    void reset_error();

    /**
     * @brief 時間を1単位進める
     * @details Errorモードなどの場合、時間経過によってLEDの数値を変化させる
     *
     * 例えばblinking_rate(2)の時は、coordinate()を2回呼ぶごとにLEDの数値が変わる
     */
    void coordinate();

    /**
     * @brief 何単位進めるとErrorモードなどが次の状態になるかを設定する
     * @param unit 何単位で進めるかの値
     */
    void blinking_rate(const uint32_t unit);

    MdLed &operator=(const State type);
           operator State() const;
    MdLed &operator=(const Mode type);
           operator Mode() const;
    MdLed &operator=(const uint32_t value);
           operator uint32_t() const;

private:
    interfaceLeds &_leds;
    State          _state;
    Mode           _mode;
    // Mutex  _mutex;
    uint32_t _interval;
    uint32_t _counter;
    uint32_t _error;

    uint32_t _error_section;
    uint32_t _error_bit_width;

    // 交互に点滅
    void alternately_blink();

    // ２つが同時に点滅
    void concurrently_blink();

    void error_blink();

    void lock();
    void unlock();

    static constexpr uint32_t default_interval = 500;
    static constexpr Mode     default_mode     = Mode::Normal;
};

}  // namespace spirit

#endif  // SPIRIT_MD_LED_H
