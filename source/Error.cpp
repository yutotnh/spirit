#include "include/Error.h"

#include "include/version.h"

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
        "%s:\n"
        "\tType: %s\n"
        "\tError code: %d\n"
        "\tMessage: %s\n"
        "\tFile: %s\n"
        "\tFunction: %s\n"
        "\tLine: %d\n"
        "\tVersion: %s\n"
        "\tCommit: %s\n"
        "\tDate: %s\n\n",
        status_to_string(_status), type_to_string(type), code, message, filename, funcname, line_number,
        SPIRIT_VERSION_STRING, SPIRIT_COMMIT_HASH, SPIRIT_COMMIT_DATE);
#else
    std::fprintf(stderr,
                 "%s:\n"
                 "\tType: %s\n"
                 "\tError code: %d\n"
                 "\tMessage: %s\n"
                 "\tFile: %s\n"
                 "\tFunction: %s\n"
                 "\tLine: %d\n"
                 "\tVersion: %s\n"
                 "\tCommit: %s\n"
                 "\tDate: %s\n\n",
                 status_to_string(_status), type_to_string(type), code, message, filename, funcname, line_number,
                 SPIRIT_VERSION_STRING, SPIRIT_COMMIT_HASH, SPIRIT_COMMIT_DATE);
    std::fflush(stderr);
#endif
}

}  // namespace spirit
