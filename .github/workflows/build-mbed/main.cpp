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

    // Mbed 用のアダプターと、それを使うライブラリ本体が、コンパイルとリンクできることを確認する
    // (ビルドの確認用なので、実行しない。ただし、コンパイラに消されないように、実行時にしか決まらない条件にしている)
    volatile bool run_adapters = false;
    if (run_adapters) {
        spirit::mbed::DigitalOut sr(LED1);
        spirit::mbed::DigitalOut reset(LED1);
        spirit::mbed::PwmOut     pwmh(LED1);
        spirit::mbed::PwmOut     pwml(LED1);
        spirit::mbed::PwmOut     phase(LED1);
        spirit::mbed::Delay      delay;

        spirit::A3921 a3921(sr, pwmh, pwml, phase, reset);
        a3921.reset(delay);

        spirit::mbed::InterruptIn a_phase(LED1);
        spirit::mbed::InterruptIn b_phase(LED1);

        spirit::SpeedController speed_controller(a_phase, b_phase);
        speed_controller.calculation(1.0F, 0.01F);

        spirit::protocol::Receiver receiver;
        uint8_t                    data[8] = {};
        receiver.receive(data, sizeof(data));
    }
}
