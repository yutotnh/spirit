# AGENTS.md

spirit は、CAN で接続したマイコン間でモータードライバーを制御するための C++14 ライブラリです。
Controller 側が `Motor` の設定をエンコードして送り、Peripheral 側（モータードライバー基板）がデコードして A3921 を駆動します。
開発は事実上停止しています（最近のコミットは Dependabot の依存更新のみ）。

## ディレクトリ構成

- `spirit.h`, `src/spirit.h`: 全ヘッダをまとめて include する傘ヘッダ
- `src/spirit/include/*.h`, `src/spirit/src/*.cpp`: 環境非依存のコア実装
- `src/spirit/platform/{arduino,mbed}/`: 実機用の `DigitalOut` / `PwmOut` / `InterruptIn` 実装（`ARDUINO` / `__MBED__` でガード）
- `tests/`: Google Test によるテスト。`stubs.h` に `InterfaceXxx` のスタブがある
- `.github/workflows/`: CI。`build-arduino-cli/` と `build-mbed/` にビルド確認用のサンプルがある
- `doxygen/`: ドキュメント生成設定（`doxygen-awesome-css` はサブモジュール）

## データの流れ

```
Motor ──protocol::encode_control()/encode_config()──> CAN のデータ部 (CANMessage, ID は can::get_motor_id)
```

受信側は `protocol::decode()` で Motor を更新します。仕様は `PROTOCOL.md`（型のプレフィックス符号、制御パケットと設定の分離、TLV の設定、前方互換の規則、ゴールデンベクタ）です。

- 型は可変長のプレフィックス符号: `0`=PWM（19bit）、`10`=Speed（20bit）、`110`=Config、`1110`=分割転送の予約、`1111…`=拡張型。どちらの制御パケットも3バイト
- 毎周期送る制御パケットに入れるのは、状態と指令値（デューティー比または速度）だけ。kp/ki/kd と `Motor` のその他の設定は、Config で送る。Controller が全設定を約100msごとに再送する（`encode_config()` の `index` と `next_index` で、1周分を何フレームかに分ける。CAN 2.0 では最大4フレーム）
- 項目は「種類・長さ(bit)・値」で、種類と長さは4bit単位の可変長整数（EVI）。未知の項目は読み飛ばし、未知の型は捨てる。長さの不整合や予約値は、フレーム全体を拒否して Motor を一切変更しない
- `decode()` は `Error` を呼ばず、結果コード（`DecodeStatus`）を返す。CAN の受信割り込みから呼べるようにするため。ただし Motor を複数の値ごと更新するので、割り込みとメインループで同じ Motor を共有してはいけない。受け渡しには `protocol::Receiver`（`Receiver.h`）を使う（割り込みで `receive()`、メインループで `snapshot()`。三重バッファでロックなし。書き手と読み手は、それぞれ1つのコンテキストだけ。`tests/test_Receiver.cpp` に2スレッドの負荷テストがあり、ThreadSanitizer でも確認済み）
- `Motor::speed()` と `pid_gain_factor()` は、無限大も拒否する（通信で送れず、受信側が拒否するため。拒否しないと、制御パケットが全て拒否されて release_time で止まる）
- release_time のタイマーをリセットしてよいのは `DecodeResult::is_control()` が true のときだけ（設定の再送ではリセットしない）
- ビット列の読み書きは `BitReader` / `BitWriter`（`BitStream.h`）。`BitWriter` はコンストラクタでバッファを 0 クリアする。`bit.h` の `get_range_value` / `set_range_value` は、範囲外のときに Error を通知するため、直接は使わない。単位は `bit.h` がバイト、プロトコルの API はバイト数（DLC）で受け取る
- `can::get_motor_id()` は `bool` を返し、ID は出力引数で受け取る（失敗時は出力を変更しない。0 は正当な ID ではないが、失敗の通知には使わない）
- `adjust_duty_cycle`（Peripheral 側）が `Motor::State` とデューティー比の急変を抑え、`A3921` が実際のピンを操作する
  - 変化の速さは「全開までの時間(s)」で設定する（`Motor::change_level()` / `full_scale_time()`）。1回あたりの最大変化幅は `Motor::get_maximum_change_duty_cycle(target, dt)` が `dt`（`adjust_duty_cycle` を呼ぶ間隔）から計算する
- `SpeedController` はエンコーダーの A/B 相割り込みから rps を求めて PID 計算する（積分は誤差×dt。積分項は出力の上限・下限で巻き上がりを防ぐ）。`limit()` を呼ぶまで出力は常に 0

## ビルドとテスト

標準の手順（README と同じ）:

```shell
cmake -S . --preset Debug
cmake --build --preset Debug
ctest --preset Debug
```

テストは `gtest_discover_tests` により **1テストごとに別プロセスで実行される**。`Error` がシングルトンで、`Error::get_status()` を検証するテストが多いため、
1プロセスで全テストを続けて実行すると、前のテストのエラー状態が残って失敗する（バグではない）。

cmake が使えない環境での代替手順（googletest 1.14.0 を別途ビルド済みとして）:

```shell
g++ -std=c++14 -g -O1 -fsanitize=address,undefined -I. -Isrc -Isrc/spirit/include -I<gtest include> \
    tests/test_Motor.cpp src/spirit/src/*.cpp <gtest-all.o> <gtest_main.o> -lpthread -o test_Motor
./test_Motor --gtest_filter='Motor.DutyCycleTest'   # 1テストずつ実行する
```

2026-11 時点で、117テストが ASan/UBSan 付きで全て通る。
`tests/test_regression.cpp` は過去に見つかった不具合の再現テスト、`tests/test_Protocol.cpp` はゴールデンベクタ・往復・前方互換・壊れたデータ・ファズ（10万回）、`test_multiple_translation_units_*.cpp` は `spirit.h` を複数の .cpp から include してもリンクできることの確認（2つの .cpp を1つの実行ファイルにする）。

## コーディング規約（CONTRIBUTING.md の要約）

- クラスと enum は PascalCase。名前空間、関数、定数は snake_case。private メンバー変数は先頭に `_`
- `if` / `for` / `while` は 1 行でも `{}` を付ける
- public な関数には Doxygen コメントを付ける
- 整形は clang-format 14（`.clang-format`、Google ベース、インデント 4、120 桁）。CI の `style-check` で検査される。cmake と同様にこの環境にない場合は、npm の `clang-format@1.8.0`（バージョン 15）でも、既存の全ファイルで差分が出ないことを確認済み
- スペルチェックは cspell（`.cspell.json`）。未登録の単語は辞書に追加する
- コメントとドキュメントは日本語
- コンパイラ警告は `-Wall -Wextra -pedantic-errors -Werror`。警告を残さない
- 異常値は `Error::get_instance().error(...)` / `warning(...)` で通知し、例外は使わない（マイコンで動かすため）。通知は `ErrorHandler`（`Error::set_handler()`）に渡され、既定では標準エラー出力に出る。テストでは `RecordingErrorHandler`（`tests/stubs.h`）で内容を検査できる。`Error` の状態は `std::atomic` で、どのスレッドからも読める。設定関数（`Motor::speed()` など）は、設定できたかどうかを `bool` で返す
- ライブラリは C++14・17・20 の全てで警告なくビルドできる。C++14 では `static constexpr` メンバーを参照渡しするとクラス外の定義が必要になるので、新しく足すときは `.cpp` に定義を書く（`tests/test_odr.cpp`）

## 注意点

- `Error` のログは `%d` に `uint32_t` を渡している箇所が多いが、現状は動作に影響しない
- `LCOV_EXCL_START` / `LCOV_EXCL_STOP` で囲んだ `default:` は、事前の検証により到達しない前提の分岐
- **プラットフォームの設計（ポートとアダプター）:** ライブラリ本体（`src/spirit/include`、`src/spirit/src`）は、`__MBED__`/`ARDUINO` の分岐も `std::function`・ミューテックスも持たない。ハードウェアは小さなインターフェースだけで受ける（`InterfaceDigitalOut`、`InterfacePwmOut`、`InterfaceInterruptIn`（割り込みは `InterruptHandler` を渡す。`nullptr` で解除）、`InterfaceDelay`）。プラットフォーム固有の処理は `src/spirit/platform/{mbed,arduino}/` のアダプターに閉じ込める。例外は、`spirit.h` がプラットフォームごとにアダプターを include する部分と、`Error.cpp` の既定の出力先（Mbed は標準出力）だけ
- アダプターは `tests/test_arduino.cpp`（`tests/arduino_stub/Arduino.h`）と `tests/test_mbed.cpp`（`tests/mbed_stub/mbed.h`）で、スタブを使ってホスト上でテストしている。ライブラリ本体はプラットフォームに依存しないので、`spirit` をそのままリンクできる（アダプターのソースだけ `ARDUINO`/`__MBED__` を定義してビルドする）。Arduino は3構成（`ARDUINO` のみ、ESP32 + core 2.x、ESP32 + core 3.x）。実機の挙動（ピンの対応、LEDC、割り込みの遅延）は確認できない
- Arduino の `attachInterrupt()` は関数ポインタしか受け取れないので、`arduino::InterruptIn` が最大8個のトランポリン関数を割り当てる
- Mbed OS は Arm が保守を終了した（2026年7月）。後継の community fork が Mbed CE。アダプターが使う API（`InterruptIn`、`PwmOut`、`DigitalOut`、`Callback`、`wait_us`）は共通のはずだが、実機・実ビルドでの確認はしていない
- 修正を入れるときは、現象を再現するテストを `tests/` に先に追加する
- 調査で見つかった不具合と修正状況は `BUGS.md` にまとめている（未修正: Arduino 関連）
- C++14 を維持する（Mbed OS 6 / Arduino のツールチェーンが C++20 に未対応）。C++14 では `static constexpr` メンバーを参照渡しすると未定義参照になるので、gtest の `EXPECT_EQ` に直接渡さずローカル変数にコピーする
