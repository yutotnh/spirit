#ifndef SPIRIT_MD_LED_H
#define SPIRIT_MD_LED_H

#include "InterfaceDigitalOut.h"
#include "Motor.h"

namespace spirit {

/**
 * @brief モーターの状態を2つのLEDを使って表示するクラス
 * @details
 * モーターの駆動状態をそのまま表示するNormalモードに加え、Errorの値を表示するErrorモードなどがある
 */
class MdLed {
public:
    /**
     * @enum BlinkMode
     * @brief LEDの点滅種類を何にするかの値
     * @details 優先度は Error > Alternate = Concurrent l Normal
     */
    enum class BlinkMode {
        /// Motor::Stateの値をそのままLEDに設定する
        Normal,
        /// LED 2つ交互に点滅 0b01 -> 0b10 -> 0b01
        Alternate,
        /// LED 2つが同じ点滅 0b11 -> 0b00 -> 0b11
        Concurrent,
        /// エラー
        Error,
    };

    /**
     * @brief コンストラクタ
     * @param [in,out] led0 下位ビットのLED
     * @param [in,out] led1 上位ビットのLED
     */
    MdLed(InterfaceDigitalOut &led0, InterfaceDigitalOut &led1);

    /**
     * @brief Normalモード時のLEDの点灯を設定する
     * @param type 設定したい値
     * @details Error中は設定値が反映されない
     */
    void state(Motor::State type);

    /**
     * @brief LEDのモードを設定する
     * @param type 設定したいLEDのモード
     */
    void mode(BlinkMode type);

    /**
     * @brief LEDをError状態にする
     * @param status Error番号
     * @details error code = 6(0b110) の場合の点滅の仕方
     *          1. 11 ... エラー番号の出力開始
     *          2. 01 ... 最下位ビットが0 -> 01
     *          3. 00 ... LEDを全てOFF
     *          4. 10 ... 下から2つ目のビットが1 -> 10
     *          5. 00 ... LEDを全てOFF
     *          6. 10 ... 最上位ビットが1 -> 10
     *          1. 11 ... エラー番号の出力開始(再)
     *          2. 01 ... 最下位ビットが0 -> 01
     *          以降繰り返し
     */
    void error(uint32_t status);

    /**
     * @brief LEDのErrorモードを解除する
     */
    void reset_error();

    /**
     * @brief 時間を1単位進め、時間経過によってLEDの数値を変化させる
     * @details 例えばblinking_rate(2)の時は、coordinate()を2回呼ぶごとにLEDの数値が変わる
     */
    void coordinate();

    /**
     * @brief 何単位進めるとErrorモードなどが次の状態になるかを設定する
     * @param unit 何単位で進めるかの値
     */
    void blinking_rate(uint32_t unit);

private:
    /// LEDの下位ビット
    InterfaceDigitalOut &_led0;
    /// LEDの上位ビット
    InterfaceDigitalOut &_led1;

    Motor::State _state{Motor::Default::state};
    BlinkMode    _mode{Default::blink_mode};
    uint32_t     _interval{Default::interval};
    uint32_t     _counter{0};
    uint32_t     _error{0};

    uint32_t _error_section{0};
    uint32_t _error_bit_width{0};

    /**
     * @brief LEDの点滅を設定する
     * @param type 2つのLEDを2bitに置き換えた場合の値
     */
    void write(uint32_t value);

    /**
     * @brief LEDの点滅状態を返す
     * @return LEDの点滅状態(範囲: 0-3)
     */
    uint32_t read() const;

    /**
     * @brief 交互に点滅 (0b10 -> 0b01 -> 0b10)
     */
    void alternately_blink();

    /**
     * @brief 同時に点滅 (0b11 -> 0b00 -> 0b11)
     */
    void concurrently_blink();

    /**
     * @brief エラーの値を表現する点滅
     */
    void error_blink();

    /**
     * @brief デフォルト値を格納している構造体
     */
    struct Default {
        static constexpr uint32_t  interval   = 500;
        static constexpr BlinkMode blink_mode = BlinkMode::Normal;
    };
};

}  // namespace spirit

#endif  // SPIRIT_MD_LED_H
