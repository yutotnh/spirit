cmake_minimum_required(VERSION 3.14)

# GoogleTestをダウンロード
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

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
        target_compile_options(${target} PUBLIC -O2 -Wall -Wextra)
    endif()

    target_include_directories(${target} PUBLIC ../include)
    target_link_libraries(${target} PRIVATE gtest_main spirit)
    gtest_discover_tests(${target})
endfunction()

# 単体テストの定義
include(GoogleTest)

# CTestへ単体テストを登録
add_gtest(test_Motor.cpp)
add_gtest(test_CANMessage.cpp)
add_gtest(test_MotorProtocolCoder.cpp)
add_gtest(test_MdLed.cpp)
add_gtest(test_A3921.cpp)
