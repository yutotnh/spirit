#include "include/Error.h"

#ifdef __MBED__
#include "mbed.h"
#else
#include <cstdio>
#endif

namespace spirit {

Error& Error::get_instance()
{
    static Error instance;
    return instance;
}

Error::Status Error::get_status()
{
    return _status;
}

void Error::warning(Type type, uint32_t code, const char* message, const char* filename, const char* funcname,
                    uint32_t line_number)
{
    _mutex.lock();
    _status = Status::Warning;
    _code   = code;
    print(type, code, message, filename, funcname, line_number);
    _mutex.unlock();
}

void Error::error(Type type, uint32_t code, const char* message, const char* filename, const char* funcname,
                  uint32_t line_number)
{
    _mutex.lock();
    _status = Status::Error;
    _code   = code;
    print(type, code, message, filename, funcname, line_number);
    _mutex.unlock();
}

void Error::print(Type type, uint32_t code, const char* message, const char* filename, const char* funcname,
                  uint32_t line_number)
{
    uint8_t status_string[10]{};
    switch (_status) {
        case Status::Normal:
            sprintf((char*)status_string, "Normal");
            break;
        case Status::Warning:
            sprintf((char*)status_string, "Warning");
            break;
        case Status::Error:
            sprintf((char*)status_string, "Error");
            break;
        default:
            Error&         error            = Error::get_instance();
            constexpr char message_format[] = "Unknown error status (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(_status));
            error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            break;
    }

    uint8_t type_string[20]{};
    switch (type) {
        case Type::Normal:
            sprintf((char*)type_string, "Normal");
            break;
        case Type::UnknownValue:
            sprintf((char*)type_string, "UnknownValue");
            break;
        case Type::IllegalCombination:
            sprintf((char*)type_string, "IllegalCombination");
            break;
        case Type::InvalidValue:
            sprintf((char*)type_string, "InvalidValue");
            break;
        default:
            Error&         error            = Error::get_instance();
            constexpr char message_format[] = "Unknown error type (%d)";
            char           message[sizeof(message_format) + Error::max_uint32_t_length];
            snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
            error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
            break;
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
        status_string, type_string, code, message, filename, funcname, line_number);
#else
    std::fprintf(stderr,
                 "%s:\n"
                 "\tType: %s\n"
                 "\tError code: %d\n"
                 "\tMessage: %s\n"
                 "\tFile: %s\n"
                 "\tFunction: %s\n"
                 "\tLine: %d\n\n",
                 status_string, type_string, code, message, filename, funcname, line_number);
    std::fflush(stderr);
#endif
}

}  // namespace spirit
