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
add_gtest(test_Protocol.cpp)
add_gtest(test_multiple_translation_units_main.cpp test_multiple_translation_units_other.cpp)
add_gtest(test_regression.cpp)
add_gtest(test_SpeedController.cpp)

# Arduino 用のコードを、 Arduino.h のスタブを使ってホスト上でテストする
# ARDUINO を定義するとライブラリ全体のヘッダの内容が変わるため、ライブラリ(spirit)とはリンクせず、必要なソースを直接ビルドする
function(add_arduino_gtest target)
    add_executable(${target}
        test_arduino.cpp
        ../src/spirit/platform/arduino/src/DigitalOut.cpp
        ../src/spirit/platform/arduino/src/PwmOut.cpp
        ../src/spirit/src/Error.cpp
        ../src/spirit/src/mutex.cpp
        ../src/spirit/src/SpeedController.cpp
    )

    # ライブラリ(spirit)とはリンクしないので、 spirit が PUBLIC に設定している /utf-8 を引き継がない
    if(MSVC)
        target_compile_options(${target} PUBLIC /W4 /WX /utf-8)
    else()
        target_compile_options(${target} PUBLIC -O2 -Wall -Wextra -pedantic-errors -Werror)
    endif()

    target_compile_definitions(${target} PRIVATE ARDUINO=10819 ${ARGN})
    target_include_directories(${target} PRIVATE arduino_stub ../src ../src/spirit/include)
    target_link_libraries(${target} PRIVATE gtest_main)
    gtest_discover_tests(${target})
endfunction()

add_arduino_gtest(test_arduino)
add_arduino_gtest(test_arduino_esp32_core2 ESP32 ESP_ARDUINO_VERSION_MAJOR=2)
add_arduino_gtest(test_arduino_esp32_core3 ESP32 ESP_ARDUINO_VERSION_MAJOR=3)
