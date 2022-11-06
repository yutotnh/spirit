cmake_minimum_required(VERSION 3.14)

project(spirit
    VERSION 0.1.0
    DESCRIPTION "モータードライバ駆動用ライブラリ"
)

add_library(spirit STATIC)

target_include_directories(spirit
    PUBLIC
    .
    ./include
)

target_sources(spirit
    PUBLIC
    source/MdLed.cpp
)

enable_testing()

add_subdirectory(tests)
