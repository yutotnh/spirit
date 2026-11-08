#ifndef SPIRIT_ERROR_H
#define SPIRIT_ERROR_H

#include <atomic>
#include <cstdarg>
#include <cstddef>
#include <cstdint>

/// 1 の場合は、エラー情報を標準エラー出力(Mbed は標準出力)に出力する既定の ErrorHandler を使う (既定値: 1) @n
/// 0 にすると、既定の ErrorHandler と、その出力のための標準入出力が不要になる (コードサイズの削減用) @n
/// その場合は、 Error::set_handler() で ErrorHandler を設定しない限り、何も出力されない
#ifndef SPIRIT_ERROR_DEFAULT_HANDLER
#define SPIRIT_ERROR_DEFAULT_HANDLER 1
#endif

namespace spirit {

/**
 * @brief エラー(警告)の情報を受け取るクラス
 * @details
 * Error::set_handler() で設定すると、エラー(警告)が通知されるたびに handle() が呼び出される @n
 * handle() は、エラー(警告)を通知した関数を呼び出したコンテキストで呼び出される @n
 * 割り込みの中で呼び出される可能性がある場合は、割り込みから呼び出せる処理(ブロックしない、ヒープを使わない) @n
 * だけを実装すること
 */
class ErrorHandler;

/**
 * @brief エラー管理クラス
 * @details
 * エラー(警告)を通知すると、エラー状態を更新し、 ErrorHandler に情報を渡す @n
 * エラー状態の更新と取得はロックなしで行うので、割り込みやほかのスレッドから呼び出してもよい
 */
class Error {
public:
    /**
     * @brief エラー状態を表すための値
     */
    enum class Status {
        /// No error
        Normal = 0,
        /// Warning
        Warning = 1,
        /// Error
        Error = 2,
    };

    enum class Type {
        /// No error
        Normal = 0,
        /// Unknown code
        UnknownValue = 1,
        /// Illegal combination
        IllegalCombination = 2,
        /// Invalid_value
        InvalidValue = 3,
    };

    /// ErrorHandler に渡すエラー(警告)の情報
    struct Info {
        /// 警告かエラーか
        Status status;
        /// 種類
        Type type;
        /// ファイル名
        const char* filename;
        /// 関数名
        const char* funcname;
        /// 行番号
        uint32_t line_number;
        /// メッセージ (書式を展開済み。 max_message_length - 1 文字までで、超える分は切り捨てる)
        const char* message;
    };

    /// メッセージの最大バッファサイズ (終端の '\0' を含む)
    static constexpr std::size_t max_message_length = 160;

    Error(const Error&)            = delete;
    Error& operator=(const Error&) = delete;
    Error(Error&&)                 = delete;
    Error& operator=(Error&&)      = delete;

    /**
     * @brief エラー管理クラスを取得する
     * @return エラー管理クラス
     */
    static Error& get_instance();

    /**
     * @brief エラー状態をリセットする
     */
    void reset();

    /**
     * @brief エラー状態を取得する
     * @details 状態は、リセットされるまで、通知された中で最も重い状態(Normal < Warning < Error)を保つ @n
     *          (エラーの後に警告を通知しても、 Error のまま)
     * @return エラー状態
     */
    Status get_status() const;

    /**
     * @brief エラー(警告)の情報を受け取る ErrorHandler を設定する
     * @param handler 設定する ErrorHandler (寿命は呼び出し側が管理する)。 nullptr の場合は、何も出力しない @n
     *                (エラー状態は更新される)
     */
    void set_handler(ErrorHandler* handler);

    /**
     * @brief 設定されている ErrorHandler を返す
     * @return 設定されている ErrorHandler (nullptr の場合は、何も出力しない)
     */
    ErrorHandler* get_handler() const;

    /**
     * @brief 標準で設定されている ErrorHandler を返す
     * @return 標準エラー出力(Mbed は標準出力)に出力する ErrorHandler。 SPIRIT_ERROR_DEFAULT_HANDLER が 0 の場合は nullptr
     */
    static ErrorHandler* default_handler();

    /**
     * @brief 警告を通知する
     * @param type 警告の種類
     * @param filename ファイル名
     * @param funcname 関数名
     * @param line_number 行番号
     * @param message 警告のメッセージ (printf 形式)
     * @param ... messageのための可変長引数
     */
    void warning(Type type, const char* filename, const char* funcname, uint32_t line_number, const char* message, ...);

    /**
     * @brief エラーを通知する
     * @param type エラーの種類
     * @param filename ファイル名
     * @param funcname 関数名
     * @param line_number 行番号
     * @param message エラーのメッセージ (printf 形式)
     * @param ... messageのための可変長引数
     */
    void error(Type type, const char* filename, const char* funcname, uint32_t line_number, const char* message, ...);

private:
    Error();
    ~Error() = default;

    /// エラー状態を更新して、 ErrorHandler に渡す
    void notify(Status status, Type type, const char* filename, const char* funcname, uint32_t line_number,
                const char* message, va_list arg);

    std::atomic<Status>        _status{Status::Normal};
    std::atomic<ErrorHandler*> _handler;
};

class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;

    /**
     * @brief エラー(警告)の情報を受け取る
     * @param info エラー(警告)の情報 (この関数の呼び出し中だけ有効)
     */
    virtual void handle(const Error::Info& info) = 0;
};

}  // namespace spirit

#endif  //  SPIRIT_ERROR_H
