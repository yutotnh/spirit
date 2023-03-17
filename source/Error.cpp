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

void Error::reset()
{
    _status = Status::Normal;
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
    auto status_to_string = [](Status status) -> const char* {
        switch (status) {
            case Status::Normal:
                return "Normal";
            case Status::Warning:
                return "Warning";
            case Status::Error:
                return "Error";
            default:
                Error&         error            = Error::get_instance();
                constexpr char message_format[] = "Unknown error status (%d)";
                char           message[sizeof(message_format) + Error::max_uint32_t_length];
                snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(status));
                error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
                return "Unknown";
        }
    };

    auto type_to_string = [](Type type) -> const char* {
        switch (type) {
            case Type::Normal:
                return "Normal";
            case Type::UnknownValue:
                return "UnknownValue";
            case Type::IllegalCombination:
                return "IllegalCombination";
            case Type::InvalidValue:
                return "InvalidValue";
            default:
                Error&         error            = Error::get_instance();
                constexpr char message_format[] = "Unknown error type (%d)";
                char           message[sizeof(message_format) + Error::max_uint32_t_length];
                snprintf(message, sizeof(message), message_format, static_cast<uint32_t>(type));
                error.error(Error::Type::UnknownValue, 0, message, __FILE__, __func__, __LINE__);
                return "Unknown";
        }
    };

#ifdef __MBED__
    printf(
        "%s:\n"
        "\tType: %s\n"
        "\tError code: %d\n"
        "\tMessage: %s\n"
        "\tFile: %s\n"
        "\tFunction: %s\n"
        "\tLine: %d\n\n",
        status_to_string(_status), type_to_string(type), code, message, filename, funcname, line_number);
#else
    std::fprintf(stderr,
                 "%s:\n"
                 "\tType: %s\n"
                 "\tError code: %d\n"
                 "\tMessage: %s\n"
                 "\tFile: %s\n"
                 "\tFunction: %s\n"
                 "\tLine: %d\n\n",
                 status_to_string(_status), type_to_string(type), code, message, filename, funcname, line_number);
    std::fflush(stderr);
#endif
}

}  // namespace spirit
