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
    ./source/A3921.cpp
    ./source/bfloat16.cpp
    ./source/bit.cpp
    ./source/CANMessage.cpp
    ./source/Error.cpp
    ./source/FakeUdpConverter.cpp
    ./source/Id.cpp
    ./source/MdLed.cpp
    ./source/Motor.cpp
    ./source/MotorDataConverter.cpp
    ./source/mutex.cpp
    ./source/PwmDataConverter.cpp
    ./source/SpeedDataConverter.cpp
)

find_package (Python3 COMPONENTS Interpreter Development)

if (NOT Python3_FOUND)
    message(FATAL_ERROR "Python3 not found")
endif()

add_custom_target(generate_version_header ALL
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_version_header.py ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/version.base.txt ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
    COMMENT "Generating version infomation"
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
        ./include
        ./platform/mbed/include
    )

    target_sources(spirit-platform-mbed
        PUBLIC
        ./platform/mbed/source/DigitalOut.cpp
        ./platform/mbed/source/PwmOut.cpp
    )

    target_link_libraries(spirit-platform-mbed spirit mbed-os)
endif()
