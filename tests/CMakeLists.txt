cmake_minimum_required(VERSION 3.14)

# GoogleTestをダウンロード
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# 親プロジェクトのコンパイラ・リンカ設定を上書きするのを防ぐ（Windowsのみ）
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

# googletestをターゲットとして使えるようにする
FetchContent_MakeAvailable(googletest)

function(add_gtest target_source)
    string(REPLACE ".cpp" "" target ${target_source})
    add_executable(${target} ${ARGV})

    if(MSVC)
        target_compile_options(${target} PUBLIC /W4 /WX)
    else()
        target_compile_options(${target} PUBLIC -O2 -Wall -Wextra -pedantic-errors -Werror)
    endif()

    target_include_directories(${target} PUBLIC ../src/spirit/include)
    target_link_libraries(${target} PRIVATE gtest_main spirit)
    gtest_discover_tests(${target})
endfunction()

# 単体テストの定義
include(GoogleTest)

# CTestへ単体テストを登録
add_gtest(integration_test.cpp)
add_gtest(test_A3921.cpp)
add_gtest(test_adjust_duty_cycle.cpp)
add_gtest(test_bfloat16.cpp)
add_gtest(test_bit.cpp)
add_gtest(test_BitStream.cpp)
add_gtest(test_CANMessage.cpp)
add_gtest(test_Error.cpp)
add_gtest(test_Id.cpp)
add_gtest(test_MdLed.cpp)
add_gtest(test_Motor.cpp)
add_gtest(test_odr.cpp)
add_gtest(test_Protocol.cpp)
add_gtest(test_Receiver.cpp)
add_gtest(test_multiple_translation_units_main.cpp test_multiple_translation_units_other.cpp)
add_gtest(test_regression.cpp)
add_gtest(test_SpeedController.cpp)
add_gtest(test_version.cpp)
target_compile_definitions(test_version PRIVATE SPIRIT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# Arduino 用のコードを、 Arduino.h のスタブを使ってホスト上でテストする
# ライブラリ本体(spirit)はプラットフォームに依存しないので、そのままリンクできる (ARDUINO を定義するのは、アダプターだけ)
function(add_arduino_gtest target)
    add_executable(${target}
        test_arduino.cpp
        ../src/spirit/platform/arduino/src/DigitalOut.cpp
        ../src/spirit/platform/arduino/src/PwmOut.cpp
        ../src/spirit/platform/arduino/src/InterruptIn.cpp
        ../src/spirit/platform/arduino/src/Delay.cpp
    )

    # ライブラリ(spirit)が PUBLIC に設定しているコンパイルオプション(MSVC の /utf-8 を含む)を引き継ぐので、 add_gtest と同様に警告の設定だけ足す
    if(MSVC)
        target_compile_options(${target} PUBLIC /W4 /WX)
    else()
        target_compile_options(${target} PUBLIC -O2 -Wall -Wextra -pedantic-errors -Werror)
    endif()

    target_compile_definitions(${target} PRIVATE ARDUINO=10819 ${ARGN})
    target_include_directories(${target} PRIVATE arduino_stub ../src ../src/spirit/include)
    target_link_libraries(${target} PRIVATE gtest_main spirit)
    gtest_discover_tests(${target})
endfunction()

add_arduino_gtest(test_arduino)
add_arduino_gtest(test_arduino_esp32_core2 ESP32 ESP_ARDUINO_VERSION_MAJOR=2)
add_arduino_gtest(test_arduino_esp32_core3 ESP32 ESP_ARDUINO_VERSION_MAJOR=3)

# 既定の ErrorHandler(標準エラー出力への出力)を含めないビルド(SPIRIT_ERROR_DEFAULT_HANDLER=0)のテスト
# Error.cpp の中身が変わるため、ライブラリ(spirit)とはリンクせず、必要なソースを直接ビルドする
add_executable(test_error_no_default_handler
    test_error_no_default_handler.cpp
    ../src/spirit/src/Error.cpp
)

if(MSVC)
    target_compile_options(test_error_no_default_handler PUBLIC /W4 /WX /utf-8)
else()
    target_compile_options(test_error_no_default_handler PUBLIC -O2 -Wall -Wextra -pedantic-errors -Werror)
endif()

target_compile_definitions(test_error_no_default_handler PRIVATE SPIRIT_ERROR_DEFAULT_HANDLER=0)
# "tests/stubs.h" を include できるように、リポジトリのルート(..)も追加する (spirit にリンクしないので、 spirit が PUBLIC に設定するルートを引き継がない)
target_include_directories(test_error_no_default_handler PRIVATE .. ../src ../src/spirit/include)
target_link_libraries(test_error_no_default_handler PRIVATE gtest_main)
gtest_discover_tests(test_error_no_default_handler)

# Mbed 用のコードを、 mbed.h のスタブを使ってホスト上でテストする
add_executable(test_mbed
    test_mbed.cpp
    ../src/spirit/platform/mbed/source/Delay.cpp
    ../src/spirit/platform/mbed/source/DigitalOut.cpp
    ../src/spirit/platform/mbed/source/InterruptIn.cpp
    ../src/spirit/platform/mbed/source/PwmOut.cpp
)

if(MSVC)
    target_compile_options(test_mbed PUBLIC /W4 /WX)
else()
    target_compile_options(test_mbed PUBLIC -O2 -Wall -Wextra -pedantic-errors -Werror)
endif()

target_compile_definitions(test_mbed PRIVATE __MBED__)
target_include_directories(test_mbed PRIVATE mbed_stub ../src ../src/spirit/include)
target_link_libraries(test_mbed PRIVATE gtest_main spirit)
gtest_discover_tests(test_mbed)
