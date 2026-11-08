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
Motor ──MotorDataConverter──> PwmDataConverter / SpeedDataConverter ──FakeUdpConverter──> CANMessage (ID は can::get_motor_id)
```

受信側は逆順にデコードします。

- 先頭 2bit が制御方式（`00`: PWM、`01`: Speed）
- PWM は 20bit、Speed は 52bit。FakeUdp が先頭に 1bit 付けるので 21bit と 53bit になる
- 単位は引数名で区別している。コンバーターと FakeUdp は `*_bits`（ビット）、`bit.h` の `get_range_value` / `set_range_value` は `buffer_bytes`（バイト）。混同しやすいので注意する
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

2026-11 時点で、97テストが ASan/UBSan 付きで全て通る。
`tests/test_regression.cpp` は過去に見つかった不具合の再現テスト、`test_multiple_translation_units_*.cpp` は `spirit.h` を複数の .cpp から include してもリンクできることの確認（2つの .cpp を1つの実行ファイルにする）。

## コーディング規約（CONTRIBUTING.md の要約）

- クラスと enum は PascalCase。名前空間、関数、定数は snake_case。private メンバー変数は先頭に `_`
- `if` / `for` / `while` は 1 行でも `{}` を付ける
- public な関数には Doxygen コメントを付ける
- 整形は clang-format 14（`.clang-format`、Google ベース、インデント 4、120 桁）。CI の `style-check` で検査される。cmake と同様にこの環境にない場合は、npm の `clang-format@1.8.0`（バージョン 15）でも、既存の全ファイルで差分が出ないことを確認済み
- スペルチェックは cspell（`.cspell.json`）。未登録の単語は辞書に追加する
- コメントとドキュメントは日本語
- コンパイラ警告は `-Wall -Wextra -pedantic-errors -Werror`。警告を残さない
- 異常値は `Error::get_instance().error(...)` / `warning(...)` で通知し、例外は使わない（マイコンで動かすため）

## 注意点

- `Error` のログは `%d` に `uint32_t` を渡している箇所が多いが、現状は動作に影響しない
- `LCOV_EXCL_START` / `LCOV_EXCL_STOP` で囲んだ `default:` は、事前の検証により到達しない前提の分岐
- Arduino 向けのコードは `tests/test_arduino.cpp` で、`tests/arduino_stub/Arduino.h` のスタブを使ってホスト上でテストしている。`ARDUINO` を定義するとヘッダの内容が変わるため、ライブラリ(`spirit`)とはリンクせず、必要なソースを直接ビルドする(`tests/CMakeLists.txt` の `add_arduino_gtest`)。構成は `ARDUINO` のみ、ESP32 + core 2.x、ESP32 + core 3.x の3つ
- Arduino では `SpeedController` のコンストラクタが割り込みを登録できないので、`attachInterrupt()` のハンドラから `a_phase_interrupt()` / `b_phase_interrupt()` を呼ぶ
- 修正を入れるときは、現象を再現するテストを `tests/` に先に追加する
- 調査で見つかった不具合と修正状況は `BUGS.md` にまとめている（未修正: Arduino 関連）
- C++14 を維持する（Mbed OS 6 / Arduino のツールチェーンが C++20 に未対応）。C++14 では `static constexpr` メンバーを参照渡しすると未定義参照になるので、gtest の `EXPECT_EQ` に直接渡さずローカル変数にコピーする
