// バージョンの手書きの重複(version.h、 CMakeLists.txt、 library.properties)が食い違っていないことの確認

#include <gtest/gtest.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include "version.h"

#ifndef SPIRIT_SOURCE_DIR
#error "SPIRIT_SOURCE_DIR must be defined (the path of the repository root)"
#endif

namespace {

std::string read_file(const std::string& relative_path)
{
    std::ifstream file(std::string(SPIRIT_SOURCE_DIR) + "/" + relative_path);
    EXPECT_TRUE(file.is_open()) << relative_path;
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string match(const std::string& text, const std::regex& pattern)
{
    std::smatch result;
    return std::regex_search(text, result, pattern) ? result[1].str() : "";
}

TEST(Version, AllSourcesAgree)
{
    const std::string header = spirit::version::string;
    const std::string cmake  = match(read_file("CMakeLists.txt"), std::regex(R"(VERSION\s+([0-9]+\.[0-9]+\.[0-9]+))"));
    const std::string properties =
        match(read_file("library.properties"), std::regex(R"(version=([0-9]+\.[0-9]+\.[0-9]+))"));

    EXPECT_FALSE(header.empty());
    EXPECT_EQ(cmake, header) << "CMakeLists.txt の project(VERSION) が version.h と違う";
    EXPECT_EQ(properties, header) << "library.properties の version が version.h と違う";
}

}  // namespace
