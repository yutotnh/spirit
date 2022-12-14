cmake_minimum_required(VERSION 3.14)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(spirit
    VERSION 0.1.0
    DESCRIPTION "モータードライバ駆動用ライブラリ"
)

add_library(spirit STATIC)

if(MSVC)
    target_compile_options(spirit PUBLIC /W4)
else()
    target_compile_options(spirit PUBLIC -O2 -Wall -Wextra)
endif()

target_include_directories(spirit
    PUBLIC
    .
    ./include
)

target_sources(spirit
    PUBLIC
    source/A3921.cpp
    source/bit.cpp
    source/CANMessage.cpp
    source/FakeUdpConverter.cpp
    source/Id.cpp
    source/MdLed.cpp
    source/Motor.cpp
    source/PwmDataConverter.cpp
)

enable_testing()

add_subdirectory(tests)
