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
    ./src
)

target_sources(spirit
    PUBLIC
    ./src/spirit/src/A3921.cpp
    ./src/spirit/src/bfloat16.cpp
    ./src/spirit/src/bit.cpp
    ./src/spirit/src/CANMessage.cpp
    ./src/spirit/src/Error.cpp
    ./src/spirit/src/FakeUdpConverter.cpp
    ./src/spirit/src/Id.cpp
    ./src/spirit/src/MdLed.cpp
    ./src/spirit/src/Motor.cpp
    ./src/spirit/src/MotorDataConverter.cpp
    ./src/spirit/src/mutex.cpp
    ./src/spirit/src/PwmDataConverter.cpp
    ./src/spirit/src/SpeedDataConverter.cpp
    ./src/spirit/src/SpeedController.cpp
)

enable_testing()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(tests)
endif()

if(DEFINED MBED_PATH)
    target_link_libraries(spirit mbed-os)

    add_library(spirit-platform-mbed STATIC)

    if(MSVC)
        target_compile_options(spirit-platform-mbed PUBLIC /W4)
    else()
        target_compile_options(spirit-platform-mbed PUBLIC -O2 -Wall -Wextra)
    endif()

    target_include_directories(spirit-platform-mbed
        PUBLIC
        .
        ./src
    )

    target_sources(spirit-platform-mbed
        PUBLIC
        ./src/spirit/platform/mbed/source/DigitalOut.cpp
        ./src/spirit/platform/mbed/source/InterruptIn.cpp
        ./src/spirit/platform/mbed/source/PwmOut.cpp
    )

    target_link_libraries(spirit-platform-mbed spirit mbed-os)
endif()
