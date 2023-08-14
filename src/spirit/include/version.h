#ifndef SPIRIT_VERSION_H
#define SPIRIT_VERSION_H

namespace spirit {

namespace version {

/// バージョン番号のメジャー番号
constexpr uint32_t major = 0;

/// バージョン番号のマイナー番号
constexpr uint32_t minor = 1;

/// バージョン番号のパッチ番号
constexpr uint32_t patch = 0;

/// バージョン番号の文字列
constexpr char string[] = "0.1.0";

}  // namespace version

}  // namespace spirit

#endif  //  SPIRIT_VERSION_H
