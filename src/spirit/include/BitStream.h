#ifndef SPIRIT_BITSTREAM_H
#define SPIRIT_BITSTREAM_H

#include <cstddef>
#include <cstdint>

namespace spirit {

/**
 * @brief カーソルを持つビット列の読み出し
 * @details
 * 先頭バイトの最上位ビットをビット位置0とする(bit.h と同じ並び) @n
 * 範囲は事前に検査するので、 bit.h のように Error を通知しない(割り込みから呼べる)
 */
class BitReader {
public:
    /**
     * @param buffer 読み出すバッファ (nullptr の場合は、長さ0として扱う)
     * @param bytes バッファのバイト数
     */
    BitReader(const uint8_t* buffer, std::size_t bytes);

    /**
     * @brief 指定したビット数を読み出して、カーソルを進める
     * @param bits 読み出すビット数 (0-32)
     * @param [out] value 読み出した値
     * @retval true 成功
     * @retval false 残りが足りない、または bits が範囲外 (カーソルは動かさない)
     */
    bool read(std::size_t bits, uint32_t& value);

    /**
     * @brief 拡張整数 (EVI) を読み出して、カーソルを進める
     * @details 4bit を1グループとし、最上位の1bitが1なら次のグループが続く。残りの3bitを連結した値になる @n
     *          最小の表現(先頭が 0b1000 でない)で、32bitに収まる値だけを受け付ける
     * @param [out] value 読み出した値
     * @retval true 成功
     * @retval false 途中で切れている、最小の表現でない、32bitを超える (カーソルは動かさない)
     */
    bool read_evi(uint32_t& value);

    /**
     * @brief 値を読まずにカーソルを進める
     * @param bits 進めるビット数
     * @retval true 成功
     * @retval false 残りが足りない (カーソルは動かさない)
     */
    bool skip(std::size_t bits);

    /**
     * @brief 残りのビット数を返す
     */
    std::size_t remaining_bits() const;

    /**
     * @brief 残りのビットが全て0(または残りがない)か
     */
    bool remaining_is_zero() const;

private:
    const uint8_t* _buffer;
    std::size_t    _bytes;
    std::size_t    _position{0};
};

/**
 * @brief ビット列の書き込み
 * @details コンストラクタでバッファを0クリアするので、使い回したバッファや未初期化のバッファに書いても、前の内容が残らない
 */
class BitWriter {
public:
    /**
     * @param buffer 書き込むバッファ (nullptr の場合は、長さ0として扱う)
     * @param bytes バッファのバイト数 (この範囲を0クリアする)
     */
    BitWriter(uint8_t* buffer, std::size_t bytes);

    /**
     * @brief 値の下位 bits ビットを書き込んで、カーソルを進める
     * @param value 書き込む値
     * @param bits 書き込むビット数 (0-32)
     * @retval true 成功
     * @retval false 残りが足りない、または bits が範囲外 (何も書き込まない)
     */
    bool write(uint32_t value, std::size_t bits);

    /**
     * @brief 拡張整数 (EVI) を書き込んで、カーソルを進める
     * @param value 書き込む値
     * @retval true 成功
     * @retval false 残りが足りない (何も書き込まない)
     */
    bool write_evi(uint32_t value);

    /**
     * @brief 書き込んだビット数を返す
     */
    std::size_t bits() const;

    /**
     * @brief 書き込んだビット数を収める最小のバイト数を返す (CANのDLCにあたる)
     */
    std::size_t bytes() const;

    /**
     * @brief 残りの書き込めるビット数を返す
     */
    std::size_t remaining_bits() const;

    /**
     * @brief 拡張整数 (EVI) で表したときのビット数を返す
     */
    static std::size_t evi_bits(uint32_t value);

private:
    uint8_t*    _buffer;
    std::size_t _bytes;
    std::size_t _position{0};
};

}  // namespace spirit

#endif  // SPIRIT_BITSTREAM_H
