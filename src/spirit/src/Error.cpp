#include "spirit/include/Error.h"

#include <cstdio>

#include "spirit/include/version.h"

namespace spirit {

#if SPIRIT_ERROR_DEFAULT_HANDLER

namespace {

/// 標準エラー出力(Mbed は標準出力)に出力する ErrorHandler
class DefaultErrorHandler : public ErrorHandler {
public:
    void handle(const Error::Info& info) override
    {
        auto status_to_string = [](Error::Status status) -> const char* {
            switch (status) {
                case Error::Status::Normal:
                    return "Normal";
                case Error::Status::Warning:
                    return "Warning";
                case Error::Status::Error:
                    return "Error";

                    // default に来ることは、 Error::warning, Error::error が直接決めていて通常の利用ではありえないため、カバレッジ計測から除外する
                    // LCOV_EXCL_START
                default:
                    return "!!! Unknown status !!!";
                    // LCOV_EXCL_STOP
            }
        };

        auto type_to_string = [](Error::Type type) -> const char* {
            switch (type) {
                case Error::Type::Normal:
                    return "Normal";
                case Error::Type::UnknownValue:
                    return "UnknownValue";
                case Error::Type::IllegalCombination:
                    return "IllegalCombination";
                case Error::Type::InvalidValue:
                    return "InvalidValue";
                default:
                    return "!!! Unknown error type !!!";
            }
        };

        // 出力が混ざらないように(ミューテックスを使わなくて済むように)、全体を1つの文字列にして、1回で出力する
        char text[max_text_length];
        std::snprintf(text, sizeof(text),
                      "%s:\n"
                      "\tType: %s\n"
                      "\tVersion: %s\n"
                      "\tFile: %s\n"
                      "\tFunction: %s\n"
                      "\tLine: %u\n"
                      "\tMessage: %s\n\n",
                      status_to_string(info.status), type_to_string(info.type), version::string, info.filename,
                      info.funcname, static_cast<unsigned int>(info.line_number), info.message);

        // Mbed は、標準出力がコンソールになる
#ifdef __MBED__
        std::fputs(text, stdout);
#else
        std::fputs(text, stderr);
        std::fflush(stderr);
#endif
    }

private:
    /// 出力する文字列の最大バッファサイズ (メッセージ + ファイル名などの付加情報)
    static constexpr std::size_t max_text_length = Error::max_message_length + 320;
};

DefaultErrorHandler& default_error_handler()
{
    static DefaultErrorHandler instance;
    return instance;
}

}  // namespace

#endif  // SPIRIT_ERROR_DEFAULT_HANDLER

Error::Error() : _handler(default_handler())
{
}

Error& Error::get_instance()
{
    static Error instance;
    return instance;
}

void Error::reset()
{
    _status.store(Status::Normal, std::memory_order_relaxed);
}

Error::Status Error::get_status() const
{
    return _status.load(std::memory_order_relaxed);
}

void Error::set_handler(ErrorHandler* handler)
{
    _handler.store(handler, std::memory_order_release);
}

ErrorHandler* Error::get_handler() const
{
    return _handler.load(std::memory_order_acquire);
}

ErrorHandler* Error::default_handler()
{
#if SPIRIT_ERROR_DEFAULT_HANDLER
    return &default_error_handler();
#else
    return nullptr;
#endif
}

void Error::warning(Type type, const char* filename, const char* funcname, uint32_t line_number, const char* message,
                    ...)
{
    va_list arg;
    va_start(arg, message);
    notify(Status::Warning, type, filename, funcname, line_number, message, arg);
    va_end(arg);
}

void Error::error(Type type, const char* filename, const char* funcname, uint32_t line_number, const char* message, ...)
{
    va_list arg;
    va_start(arg, message);
    notify(Status::Error, type, filename, funcname, line_number, message, arg);
    va_end(arg);
}

void Error::notify(Status status, Type type, const char* filename, const char* funcname, uint32_t line_number,
                   const char* message, va_list arg)
{
    // 状態は、重い方(Error > Warning > Normal)に更新する。エラーの後に警告が通知されても、 Error のままにする
    Status current = _status.load(std::memory_order_relaxed);
    while (static_cast<int>(current) < static_cast<int>(status) &&
           !_status.compare_exchange_weak(current, status, std::memory_order_relaxed)) {
    }

    ErrorHandler* handler = get_handler();
    if (handler == nullptr) {
        return;
    }

    // ErrorHandler には、書式を展開したメッセージを渡す
    char buffer[max_message_length];
    std::vsnprintf(buffer, sizeof(buffer), message, arg);

    Info info;
    info.status      = status;
    info.type        = type;
    info.filename    = filename;
    info.funcname    = funcname;
    info.line_number = line_number;
    info.message     = buffer;
    handler->handle(info);
}

}  // namespace spirit
