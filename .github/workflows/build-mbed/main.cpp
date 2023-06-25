#include "mbed.h"
#include "spirit.h"

static constexpr uint32_t pc_baud = 115'200;

int main()
{
    // 念のため、mbedのライブラリを使うことができることを確認する
    BufferedSerial pc(USBTX, USBRX, pc_baud);

    // とりあえず、spirit.h の中の簡単に呼べる関数を呼んで
    // ビルドが上手くいっていることを確認する
    spirit::bfloat16_to_float32(123);
}
