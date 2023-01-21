#ifndef SPIRIT_ERROR_H
#define SPIRIT_ERROR_H

#include <atomic>
#include <cstdint>

#include "include/mutex.h"

namespace spirit {

// void error(const char* error_msg, const char* filename, const char* funcname, uint32_t line_number);

/**
 * @brief エラー管理クラス
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
     * @brief エラー状態を取得する
     * @return エラー状態
     */
    Status get_status();

    /**
     * @brief エラーコードを取得する
     * @return エラーコード
     */
    uint32_t get_code();

    /**
     * @brief 警告情報の出力と情報の保存を行う
     * @param type エラー(警告)の種類
     * @param code エラー(警告)の値
     * @param msg エラー(警告)のメッセージ
     * @param filename ファイル名
     * @param funcname 関数名
     * @param line_number 行番号
     */
    void warning(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
                 uint32_t line_number);

    /**
     * @brief エラー情報の出力と情報の保存を行う
     * @param type エラー(警告)の種類
     * @param code エラー(警告)の値
     * @param msg エラー(警告)のメッセージ
     * @param filename ファイル名
     * @param funcname 関数名
     * @param line_number 行番号
     */
    void error(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
               uint32_t line_number);

private:
    Error()  = default;
    ~Error() = default;

    spirit::mutex _mutex;
    Status        _status{Status::Normal};
    uint32_t      _code{0};

    /**
     * @brief エラー(警告)情報を出力する
     * @param type エラー(警告)の種類
     * @param code エラー(警告)の値
     * @param msg エラー(警告)のメッセージ
     * @param filename ファイル名
     * @param funcname 関数名
     * @param line_number 行番号
     */
    void print(Type type, uint32_t code, const char* msg, const char* filename, const char* funcname,
               uint32_t line_number);
};

}  // namespace spirit

#endif  //  SPIRIT_ERROR_H
