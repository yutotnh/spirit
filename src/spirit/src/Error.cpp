#include "spirit/include/Error.h"

#include "cstdarg"

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

void Error::warning(Type type, uint32_t code, const char* filename, const char* funcname, uint32_t line_number,
                    const char* message, ...)
{
    _mutex.lock();
    _status = Status::Warning;
    _code   = code;

    va_list arg;
    va_start(arg, message);
    print(type, code, filename, funcname, line_number, message, arg);
    va_end(arg);

    _mutex.unlock();
}

void Error::error(Type type, uint32_t code, const char* filename, const char* funcname, uint32_t line_number,
                  const char* message, ...)
{
    _mutex.lock();
    _status = Status::Error;
    _code   = code;

    va_list arg;
    va_start(arg, message);
    print(type, code, filename, funcname, line_number, message, arg);
    va_end(arg);
    _mutex.unlock();
}

void Error::print(Type type, uint32_t code, const char* filename, const char* funcname, uint32_t line_number,
                  const char* message, va_list arg)
{
    auto status_to_string = [](Status status) -> const char* {
        switch (status) {
            case Status::Normal:
                return "Normal";
            case Status::Warning:
                return "Warning";
            case Status::Error:
                return "Error";

                // default に来ることは、print を呼び出す関数が直接決めていて通常の利用ではありえないため、カバレッジ計測から除外する
                // LCOV_EXCL_START
            default:
                return "!!! Unknown status !!!";
                // LCOV_EXCL_STOP
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
                // mutex.lock() 中で error() は使えないので、直接値を書き込む
                return "!!! Unknown error type !!!";
        }
    };

#ifdef __MBED__
    printf(
#else
    std::fprintf(stderr,
#endif  // __MBED__
        "%s:\n"
        "\tType: %s\n"
        "\tError code: %d\n"
        "\tFile: %s\n"
        "\tFunction: %s\n"
        "\tLine: %d\n"
        "\tMessage: ",
        status_to_string(_status), type_to_string(type), code, filename, funcname, line_number);

#ifdef __MBED__
    vprintf(stderr, message, arg);
    printf("\n\n");
#else
    std::vfprintf(stderr, message, arg);
    std::fprintf(stderr, "\n\n");
#endif

#ifndef __MBED__
    std::fflush(stderr);
#endif  // !__MBED__
}

}  // namespace spirit
