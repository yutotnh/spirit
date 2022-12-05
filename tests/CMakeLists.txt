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

# 単体テストの定義
add_executable(test_motor test_Motor.cpp)
target_include_directories(test_motor PUBLIC ../include)
target_link_libraries(test_motor PRIVATE gtest_main spirit)

add_executable(test_mdled test_MdLed.cpp)
target_include_directories(test_mdled PUBLIC ../include)
target_link_libraries(test_mdled PRIVATE gtest_main spirit)

add_executable(test_a3921 test_A3921.cpp)
target_include_directories(test_mdled PUBLIC ../include)
target_link_libraries(test_a3921 PRIVATE gtest_main spirit)

# CTestへ単体テストを登録
include(GoogleTest)
gtest_discover_tests(test_motor)
gtest_discover_tests(test_mdled)
gtest_discover_tests(test_a3921)
