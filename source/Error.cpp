#include "Error.h"

#ifdef __MBED__
#include "mbed.h"
#else
#include <iostream>
#endif

namespace spirit {

// namespace {

// void print(const char* error_msg, const char* filename, const char* funcname, const uint32_t line_number)
// {
// #ifdef __MBED__
//     printf("Error: %s, %s, %s, %d\n", error_msg, filename, funcname, line_number);
// #else
//     std::cout << "Error: " << error_msg << ", " << filename << ", " << funcname << ", " << line_number << std::endl;
// #endif
// }

// }  // namespace

// void error(uint32_t type, uint32_t code, const char* msg, const char* filename, const char* funcname,
//            uint32_t line_number)
// {
//     print(error_msg, filename, funcname, line_number);
// }

Error& Error::get_instance()
{
    static Error instance;
    return instance;
}

Error::Status Error::get_status()
{
    return _status;
}

void Error::warning(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
                    uint32_t line_number)
{
    _mutex.lock();
    _status = Status::Warning;
    _code   = code;
    print(type, code, msg, filename, funcname, line_number);
    _mutex.unlock();
}

void Error::error(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
                  uint32_t line_number)
{
    _mutex.lock();
    _status = Status::Error;
    _code   = code;
    print(type, code, msg, filename, funcname, line_number);
    _mutex.unlock();
}

void Error::print(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
                  uint32_t line_number)
{
    uint8_t status_string[10]{};
    if (_status == Status::Normal) {
        sprintf((char*)status_string, "Normal");
    } else if (_status == Status::Warning) {
        sprintf((char*)status_string, "Warning");
    } else if (_status == Status::Error) {
        sprintf((char*)status_string, "Error");
    }

    uint8_t type_string[20]{};
    if (type == Type::Normal) {
        sprintf((char*)type_string, "Normal");
    } else if (type == Type::UnknownValue) {
        sprintf((char*)type_string, "UnknownValue");
    } else if (type == Type::IllegalCombination) {
        sprintf((char*)type_string, "IllegalCombination");
    } else if (type == Type::InvalidValue) {
        sprintf((char*)type_string, "InvalidValue");
    }

#ifdef __MBED__
    printf(
        "%s:\n"
        "\tType: %s\n"
        "\tError code: %d\n"
        "\tMessage: %s\n"
        "\tFile: %s\n"
        "\tFunction: %s\n"
        "\tLine: %d\n\n",
        status_string, type_string, code, msg, filename, funcname, line_number);
#else
    std::fprintf(stderr,
                 "%s:\n"
                 "\tType: %s\n"
                 "\tError code: %d\n"
                 "\tMessage: %s\n"
                 "\tFile: %s\n"
                 "\tFunction: %s\n"
                 "\tLine: %d\n\n",
                 status_string, type_string, code, msg, filename, funcname, line_number);
    std::fflush(stderr);
#endif
}

}  // namespace spirit
