# 調査で見つかった不具合（2026-11-08 時点）

ソースを全て読み、既存テスト61件を ASan/UBSan 付きで実行し、疑わしい箇所は再現プログラムで確認しました。
「再現済み」は実際に再現した項目、「推測」はコードを読んだだけの項目です。

## 修正状況

下の #0〜#8 は修正済みです。再現テストを `tests/test_regression.cpp` と `tests/test_multiple_translation_units_*.cpp` に追加しました
（修正前は失敗し、修正後は通ることを確認）。修正後は既存を含めて97テストが全て通ります（ASan/UBSan 付き）。

| 項目 | 状態 |
| --- | --- |
| #0 Coast が送れない | 修正済み。`encode()` が使うバイトを先に 0 クリアし、Coast も状態ビットに明示的に 0 を書く |
| #1 `spirit.h` の多重定義 | 修正済み。`inline` を付け、`Error.h` を include |
| #2 起動直後の rps | 修正済み。一周目にも角度を記録する |
| #3 Speed のサイズ検査 | 修正済み。52bit で検査。不正な値（負の速度・kp・ki）は何も更新せず `false` を返す。空のバッファも拒否 |
| #4 `MdLed::mode(Normal)` | 修正済み。`mode(Error)` のエラーは仕様どおり（既存テストが期待）で、メッセージだけ `error()` を案内する内容に変更 |
| #5 `MdLed::state()` | 修正済み。点滅中は回転方向を保持するだけ。不正な値は保持しない |
| #6 Middle と High が同じ値 | 修正済み。変化幅を「1回あたり」ではなく「全開までの時間(s)」で定義し直した（OFF=0、Low=0.1、Middle=0.5、High=1、Max=3 秒）。`Motor::get_maximum_change_duty_cycle(target, dt)` は `dt / 時間` を返す。`Manual` も秒で指定する（`Motor::full_scale_time()`）。値は目安なので、実機で調整が必要 |
| #7 bfloat16 の極小値 | 修正済み。2^-126 未満は符号付きの 0 にする。非正規化数のデコードも修正 |
| #8 積分の巻き上がり | 修正済み。積分項が上限・下限に達したら誤差を蓄積しない |
| 仕様と実装の食い違い | `Motor.h` のドキュメントを実装に合わせて修正。`kd` は受信で上書きせず、Motor の値を維持するよう変更 |
| Arduino: `SpeedController` の割り込み | 修正済み（API 追加）。Arduino は `attachInterrupt()` にインスタンスを渡せないため、`a_phase_interrupt()` / `b_phase_interrupt()` を public にし、ハンドラから呼ぶ形にした |
| Arduino: `PwmOut` | 修正済み。ESP32 は `dacWrite`（DAC）をやめ、LEDC（ハードウェア PWM）を使う。Arduino core 2.x（チャンネル指定）と 3.x（ピン指定）の両方に対応し、`period()` で周波数を変更できる。それ以外のボードは `analogWrite()` のままで、周期は変更できない。範囲外の値はクランプしてから変換する |
| NaN が通る箇所 | 修正済み。`Motor` の各設定、`A3921::duty_cycle`、`SpeedController::calculation` が NaN を拒否する |
| デューティー比の16bit化 | 修正済み。切り捨てから四捨五入にした（誤差は最大 約7.6e-6 で、以前の約1.5e-5 の半分） |
| 微分項のキック | 修正済み。微分を誤差ではなく測定値（rps）に対して行う |
| ログの書式 `"% zu"` | 修正済み |
| `SpeedController` の `dt` の扱いと `/ 2.0f`、`volatile` | 修正済み。後方互換が不要なため、積分を `誤差 × dt` にし、`/ 2.0f` を外した（同じ ki でも積分の効きが以前の2倍になる）。`_angle_counter` に `volatile` を付けた |
| `buffer_size` の単位の混同 | 修正済み（引数名）。コンバーターと FakeUdp は `*_bits`、`bit.h` は `buffer_bytes` に改名 |
| `get_motor_id` が失敗を 0 で返す | 修正済み。`bool get_motor_id(..., uint32_t& motor_id)` に変更し、失敗時は出力を変更しない |

ライブラリは Mbed OS 6 / Arduino をターゲットにしており、これらのツールチェーンが C++20 に対応していないため、C++14 のままにしています。

## 確実に不具合（再現済み）

### 0. 送信バッファを使い回すと Coast が送れず、直前の回転方向が残る（重大度: 高）

`PwmDataConverter::set_state` と `SpeedDataConverter::set_state` は、`Coast` のとき「既に0で初期化されているので不要」として何も書きません。
`encode()` が書き込むのは先頭バイトと値の部分だけなので、状態ビット（PWM は 18–19 bit、Speed は 50–51 bit）には前回の値が残ります。
呼び出し側が `encode()` に渡すバッファを 0 クリアする約束はどこにも書かれていません（Doxygen の記述なし）。

- バッファを使い回して CW → Coast と encode すると、受信側は CW のままで、モーターは止まらず回り続けます（PWM / Speed とも再現）
- 初期化していない配列（スタックの残り物）や 0xFF 埋めのバッファで Coast を encode すると、受信側は Brake（3）になります
- 既存の結合テストは毎回 `= {}` で初期化しているため、見つかっていません
- 再現: `memset(buf, 0, 8)` → state=CW で encode → 同じ `buf` に state=Coast で encode → decode して `get_state()` を見る
- 修正案: `encode()` の先頭で `buffer` の必要バイトを 0 クリアするか、`Coast` の場合も `set_range_value(0x00, ...)` で明示的に書く

クライアントが送信ループでバッファを使い回すのは自然な書き方なので、「実際に動かすと一部おかしかった」現象の最有力候補です。

### 1. `spirit.h` を複数の .cpp で include するとリンクエラー（重大度: 高）

`src/spirit/include/adjust_duty_cycle.h` が `inline` なしで関数の本体を定義しています。
傘ヘッダ `spirit.h` がこれを include するので、クライアントが `spirit.h` を2つ以上の .cpp で include すると
`multiple definition of spirit::adjust_duty_cycle(...)` でリンクできません。
テストが通っていたのは、1テストにつき .cpp が1つだけだったためです。
さらにこのヘッダは `Error` を使っているのに `Error.h` を include しておらず、単独で include するとコンパイルエラーになります
（`spirit.h` 経由では、先に `Error.h` が include されているので通ります。include 順への依存です）。

- 再現: `#include "spirit.h"` だけの .cpp を2つ作ってリンクする
- 修正案: `inline` を付けるか、本体を .cpp に移して CMake の `target_sources` と Arduino のビルドに加える。`#include "Error.h"` も足す

### 2. `SpeedController` の rps が起動直後に約1.1〜1.9倍に跳ねる（重大度: 高）

`rps_calculation()` の最初の一周（`first_loop`）では `_angle_buff` に角度を保存しません。
そのため二周目（index 1〜9）の差分が、バッファに残った 0 との差になり、「リセット後の総回転量 ÷ 10周期分の時間」になります。
1 rps で一定回転させても、起動後 11〜19 回目の計算で 1.1, 1.2, … 1.9 rps と出ます（20回目から正常な 1.0）。
PID の入力が一時的に過大になるので、立ち上がりでオーバーシュートやガタつきが出ます。
クライアントのサンプルで速度制御がおかしく見えた原因の候補です。

- 再現: 8カウント/周期で 25 回 `calculation(1.0f, 0.01f)` を呼び、`rps()` を並べる
- 修正案: 一周目にも `_angle_buff[index] = _angle_counter` を保存する

### 3. `SpeedDataConverter::decode` のサイズ検査の単位が違い、短いフレームで境界外を読む（重大度: 中）

`buffer_size < 7` と比較していますが、`buffer_size` はビット数で、Speed の必要量は 52 です（`encode` は 52 を返す）。
7ビットのバッファでも受理され、内部では 7 バイト分を読むため、ASan が heap-buffer-overflow を報告します。
Peripheral の受信バッファが固定長配列なら、切れたフレームを受け取っても読み出しは安全ですが、
古いデータを混ぜてデコードしてしまいます。PWM 側は `20` で正しく検査しています。

- 再現: 1バイトのヒープ領域に `0x40` を置いて `SpeedDataConverter().decode(buf, 7, motor)` を呼ぶ
- 修正案: `7` を `52` に直す

### 4. `MdLed::mode(Normal)` が必ず Error ログを出す（重大度: 中）

`mode()` の2つ目の `switch` が `Alternate` と `Concurrent` しか扱わず、`Normal` が `default:` に落ちて
`Unknown blink mode` を `error()` で報告します。`Error::get_status()` が `Error` になるため、
状態を見ているクライアントは誤って異常と判断します。LED の動作自体は設定されます。
（`mode(Error)` がエラーになるのは仕様です。エラー番号が必要なので `error()` で設定します。）

- 再現: `mode(Alternate)` のあとに `mode(Normal)` を呼ぶ
- 修正案: 2つ目の `switch` に `Normal` / `Error` の `case` を追加して `break` する

### 5. `MdLed::state()` が Alternate/Concurrent モードを黙って上書きする（重大度: 低〜中）

ヘッダのコメントには「優先度は Error > Alternate = Concurrent > Normal」とありますが、
`state()` は Error 以外のとき `_mode = Normal` にします。`mode(Alternate)` の後にモーターの状態を更新すると、点滅が止まります。
仕様の解釈によるので、意図どおりならコメントの修正で足ります。

### 6. `Motor::ChangeLevel::Middle` と `High` が同じ値（重大度: 低〜中。修正済み）

`get_maximum_change_duty_cycle()` が `Middle` と `High` の両方で 0.0001 を返します。
`Low`=0.001、`Max`=0.00001 と並べると、`High` は 0.00005 程度のはずで、コピーミスの可能性が高いです。
ただし正しい値が分からないため、作者に確認が必要です。

### 7. `float32_to_bfloat16` が極小値で符号と値を壊す（重大度: 低）

絶対値が約 5.9e-39（2^-127）未満のとき、指数部が負になって `uint32_t` でラップし、`1e-40f` が `-1.15e37` に化けます。
通常の速度やゲインでは到達しません。`bfloat16_to_float32` も非正規化数に暗黙の 1 を足します。
なお変換は丸めではなく切り捨てで、相対誤差は最大で約 0.8% です（0.3 → 0.2988、65504 → 65280）。速度や kp/ki の精度の目安になります。

### 8. `SpeedController` の積分項が巻き上がる（重大度: 中。設計上の問題）

`_sum_error` に上限がなく、クランプされるのは出力 `i` だけです。モーターが回らない状態（拘束、エンコーダー断線など）で
目標を与え続けると誤差が溜まり続け、そのあと回り始めて誤差が逆向きになっても、積分項が上限から戻るまでに長い時間がかかります。
（誤差が 0 のままなら、積分項が保持されるのは PI 制御として正常な動作です。最初の調査で書いた「目標を 0 にしても張り付く」は観測の誤りでした。）
再現テストでは、3000 周期の無回転のあとに目標の1.5倍で回したとき、修正前は出力が下がるまで数千周期かかります。
また `_sum_error` は `dt` を掛けずに加算し、あとで現在の `dt` を掛けるため、`dt` が変動すると積分が合いません。`/ 2.0f` により実効的な ki が半分になっています。
`dt` を掛けずに加算する点と `/ 2.0f` も、のちに修正しました（上の表を参照）。

## 仕様と実装の食い違い（要確認）

- `Motor::pulse_period` のドキュメントは「範囲を超えると端に設定される」ですが、実装はエラーを出して**変更しません**。範囲の記述（`0.1s <= seconds <= 1.6666e-5s`）も上下が逆です
- `Motor::release_time` のドキュメントは「0.0s より小さい場合は停止しない」ですが、実装は負数をエラーとして**拒否**します
- `SpeedDataConverter::decode` は不正な値（負の rps）を検出しても `true` を返します。そのとき制御方式と PID ゲインは更新済みで、速度だけ古い値のままです
- `SpeedDataConverter` は `kd` を送らないので、受信側で `kd` が常に 0 になります（`Default::kd` = 0.2 を上書き）。ただし `SpeedController` と `Motor` のどちらが `kd` を使うかは、このリポジトリからは分かりません
- `SpeedController` は `limit()` を呼ぶまで上下限が 0/0 で、`calculation()` が常に 0 を返します。ドキュメントに記載がなく、サンプルで「動かない」と見える落とし穴です
- `MotorDataConverter::decode` は `buffer_size` を確認する前に `buffer[0]` を読みます（サイズ 0 や `nullptr` に弱い）

## 実機（Arduino）での不具合の疑い（修正済み。ただし実機は未確認）

- `SpeedController` は Arduino 用の割り込み登録が `TODO` のままなので、Arduino では A/B 相を一切数えず、速度制御が動きません（コンパイルは通ります）
- `arduino::PwmOut::period()` が未実装なので、`Motor::pulse_period` の設定が反映されません
- ESP32 の `arduino::PwmOut::write()` が `dacWrite()` を使っています。`dacWrite` は PWM ではなく DAC で、対応ピンも GPIO25/26 に限られます
- `PwmOut::write()` は範囲外の値をクランプする前に `value * 255` を整数に変換して書き出しています
- `SpeedController::_angle_counter` は割り込みと本体の両方から更新・参照されますが、`volatile` も atomic も付いていません（最適化で読み出しが省かれる可能性）
- Mbed では `Error::error()` が `Mutex` と `printf` を使います。Ticker や CAN のコールバック（割り込みコンテキスト）から呼ぶと Mbed OS が停止する恐れがあります。#4 のせいで `MdLed::mode(Normal)` でも `error()` に入ります

## クライアントのサンプルについて

リポジトリ内のサンプル（`.github/workflows/build-arduino-cli/*.ino` と `build-mbed/main.cpp`）は、`bfloat16_to_float32(123)` を1回呼ぶだけのビルド確認用で、
動作確認には使えません（作業ディレクトリ以外にも、未追跡のサンプルや、履歴に残る削除済みのサンプルはありませんでした）。
「サンプルで動かしたら一部動作がおかしかった」というご記憶は、別のリポジトリのクライアントのことかもしれません。
上の #0（バッファ使い回し）、#2（速度の跳ね上がり）、#8（積分の巻き上がり）、`limit()` を呼ばないと出力が 0 のままになる件が、
実機で起こりうる現象として該当しそうです。

## 問題なしと確認した項目

- `FakeUdpConverter` の往復（1〜63ビット × 各50パターン）、`get_motor_id` の範囲（11bit 以内）と一意性
  （モーター数3と4でIDが重なるのは仕様。基板は3か4のどちらかしか持たない）
- PWM と Speed を Motor → CAN 相当のバイト列 → Motor と往復させたときの値（Speed: 12.5 rps / kp 1.5 / ki 0.25 / 状態）
- 既存テスト61件は全て通る（ASan/UBSan 付き、1テストずつ実行）。この環境に cmake がないため、公式の `cmake` / `ctest` は実行せず、g++ で手動ビルドした
- CI と同じ `-O2 -Wall -Wextra -pedantic-errors -Werror`（GCC 15、C++14）で、ライブラリのソースは警告なしでコンパイルできる
- PWM のデューティー比は 16bit に切り捨てるため、最大で約 1.5e-5 の誤差が出る（0.25 → 0.249989）。実用上は問題なし

## 未解決・実機での確認が必要なもの

- **Arduino の実機動作:** `tests/test_arduino.cpp` は `Arduino.h` のスタブでホスト上のテストをしており、確認できるのは「core の API を想定どおりの順序・値で呼んでいること」と変換ロジックです。実際のピンの対応、LEDC の周波数と分解能の上限、`analogWrite()` の周波数は、実機で確認してください。
- **`kd` は送られない:** 速度制御のパケットは 52bit で、`kd`（16bit）を足すと CAN の 64bit を超えます。プロトコルの変更が必要なため、受信側の Motor が持つ値を維持する動作のままです。
- **`ChangeLevel` の時間（0.1 / 0.5 / 1 / 3 秒）と PID のゲイン:** 目安です。実機で調整してください。
- **Mbed で割り込みコンテキストから `Error::error()` を呼ばないこと:** `Mutex` と `printf` を使うため。ライブラリ自身の割り込みハンドラは `Error` を呼びません。

## 追記: プロトコルの再設計

上の #0、#3 と、`kd` が送れない問題は、プロトコルの再設計（`PROTOCOL.md`）で、問題のあったコンバーターごと置き換えました。
`PwmDataConverter`、`SpeedDataConverter`、`MotorDataConverter`、`FakeUdpConverter` は削除し、`protocol::encode_control()`、`encode_config()`、`decode()` に置き換えています。
`kd` を含む `Motor` の全設定が送れるようになり、未知の項目・型や壊れたデータは、`Motor` を変更せずに拒否します。
旧テストの `integration_test` には、速度の確認が受信側ではなく送信側の `Motor` を参照している誤りがありました（新しいテストでは受信側と比較しています）。

## 設計上の妥協（調査結果と対応状況）

実際にコードを読み、疑わしい点は再現して確認しました。

| 項目 | 状態 |
| --- | --- |
| `Error` のシングルトンで、どの呼び出しが失敗したか分からない | 修正済み。`Motor`、`A3921`、`MdLed`、`SpeedController::limit` の設定関数が `bool` を返す（設定できたかどうか）。`Error` はグローバルな診断のまま残す |
| `get_status()` と `reset()` がロックなし（データ競合） | 修正済み。状態を `std::atomic` にした（ThreadSanitizer で確認）。状態は、リセットされるまで最も重いもの（Error > Warning）を保つ |
| `code` 引数が全65か所で 0 | 修正済み。引数と `get_code()` を削除した |
| 出力先が `stderr`/`printf` に固定 | 修正済み。`ErrorHandler`（DI）を `Error::set_handler()` で差し替えられる。`SPIRIT_ERROR_DEFAULT_HANDLER=0` でビルドすると、既定の出力と標準入出力が不要になる（記号が消えることを確認）。`tests/stubs.h` に、内容を記録する `RecordingErrorHandler` がある |
| `spirit::mutex` が STM32/Mbed 版の Arduino で空 | `Error` の状態更新からミューテックスを外した。残るのは、既定の出力ハンドラ（出力が混ざらないようにする用途）だけ。割り込みから通知する場合は、別の `ErrorHandler` を設定すること（`Error.h` に記載） |
| C++14 の ODR | 修正済み。`Motor`、`CANMessage`、`MdLed`、`SpeedController` にクラス外の定義を追加し、`tests/test_odr.cpp` で参照渡しでもリンクできることを確認 |
| `CANMessage` のバッファ溢れと `nullptr` | 修正済み。`get_data(buffer, buffer_size)` に変更し、`data` が `nullptr` なら長さ0のメッセージにする |
| `SpeedController` の `limit()` 未設定で出力 0、PPR 固定 | 修正済み。既定の上下限を 1.0 / 0.0 にし、PPR をコンストラクタの引数にした。割り込みと共有するカウンターは `std::atomic`（`volatile` は C++20 で非推奨のため） |
| バージョンの重複 | 食い違いを検出するテストを追加（`tests/test_version.cpp`）。3か所の手書きは残したまま |
| プラットフォームの抽象化（`__MBED__`/`ARDUINO` の分岐、`InterfaceInterruptIn`） | **未対応**（設計が必要） |
| `std::function`（`A3921::reset()` など） | **未対応** |
| `Error` の `spirit::mutex` 自体（STM32/Mbed 版の Arduino で空） | **未対応**（`Error` は使わなくなったが、`mutex` クラスは残っている） |

ライブラリは、C++14・17・20 のいずれでも、`-Wall -Wextra -pedantic -Werror` で警告なくコンパイルできることを確認しました（g++ 15）。
