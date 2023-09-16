cmake_minimum_required(VERSION 3.14)

# GoogleTestをダウンロード
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

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
        target_compile_options(${target} PUBLIC /W4)
    else()
        target_compile_options(${target} PUBLIC -O2 -Wall -Wextra)
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
add_gtest(test_CANMessage.cpp)
add_gtest(test_Error.cpp)
add_gtest(test_FakeUdpConverter.cpp)
add_gtest(test_Id.cpp)
add_gtest(test_MdLed.cpp)
add_gtest(test_Motor.cpp)
add_gtest(test_MotorDataConverter.cpp)
add_gtest(test_PwmDataConverter.cpp)
add_gtest(test_SpeedDataConverter.cpp)
add_gtest(test_SpeedController.cpp)
