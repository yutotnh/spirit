#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "Error.h"
#include "Id.h"
#include "tests/stubs.h"

namespace {

using namespace spirit;

/// 各テストの前後で、状態と ErrorHandler を元に戻す
class ErrorTest : public testing::Test {
protected:
    void SetUp() override
    {
        Error::get_instance().set_handler(Error::default_handler());
        Error::get_instance().reset();
    }

    void TearDown() override
    {
        Error::get_instance().set_handler(Error::default_handler());
        Error::get_instance().reset();
    }
};

/**
 * @brief Errorクラスのテスト
 */
TEST_F(ErrorTest, StatusTest)
{
    // Error時に標準エラー出力に文字列が出力される
    // 本当のエラー時にエラー出力させたいので、異常系のテスト中は標準エラー出力をキャプチャする
    testing::internal::CaptureStderr();

    spirit::Error& error = spirit::Error::get_instance();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    /// @test warning が呼ばれたら、エラー状態が Warning になる
    error.warning(spirit::Error::Type::UnknownValue, __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Warning);

    /// @test error が呼ばれたら、エラー状態が Error になる
    error.error(spirit::Error::Type::UnknownValue, __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Error);

    /// @test reset が呼ばれたら、エラー状態が Normal になる
    error.reset();
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Normal);

    /// 異常系

    /// @test spirit::Error::Type が範囲外の時、エラー状態が Error になる
    error.error(static_cast<spirit::Error::Type>(4), __FILE__, __func__, __LINE__, "test");
    EXPECT_EQ(error.get_status(), spirit::Error::Status::Error);

    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Type: !!! Unknown error type !!!"), std::string::npos);
}

/// エラーの後に警告が通知されても、リセットされるまで Error のままであることの確認
TEST_F(ErrorTest, StatusNeverDowngrades)
{
    Error& error = Error::get_instance();
    error.set_handler(nullptr);

    error.error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "error");
    error.warning(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "warning");
    EXPECT_EQ(error.get_status(), Error::Status::Error);

    error.reset();
    error.warning(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "warning");
    EXPECT_EQ(error.get_status(), Error::Status::Warning);
}

/// 標準で設定される ErrorHandler が、従来と同じ形式で標準エラー出力に出力することの確認
TEST_F(ErrorTest, DefaultHandlerPrintsToStderr)
{
    ASSERT_NE(Error::default_handler(), nullptr);
    EXPECT_EQ(Error::get_instance().get_handler(), Error::default_handler());

    testing::internal::CaptureStderr();
    Error::get_instance().error(Error::Type::InvalidValue, "file.cpp", "function", 123, "value (%d) is %s", 42, "bad");
    const std::string output = testing::internal::GetCapturedStderr();

    EXPECT_NE(output.find("Error:\n"), std::string::npos);
    EXPECT_NE(output.find("\tType: InvalidValue\n"), std::string::npos);
    EXPECT_NE(output.find("\tFile: file.cpp\n"), std::string::npos);
    EXPECT_NE(output.find("\tFunction: function\n"), std::string::npos);
    EXPECT_NE(output.find("\tLine: 123\n"), std::string::npos);
    EXPECT_NE(output.find("\tMessage: value (42) is bad\n"), std::string::npos);
}

/// ErrorHandler を設定すると、標準エラー出力ではなく、 ErrorHandler にエラーの内容が渡されることの確認 (DI)
TEST_F(ErrorTest, InjectedHandlerReceivesErrors)
{
    RecordingErrorHandler handler;
    Error&                error = Error::get_instance();
    error.set_handler(&handler);
    EXPECT_EQ(error.get_handler(), &handler);

    testing::internal::CaptureStderr();
    error.warning(Error::Type::IllegalCombination, "a.cpp", "func_a", 10, "warning %d", 1);
    error.error(Error::Type::UnknownValue, "b.cpp", "func_b", 20, "error %s", "x");
    EXPECT_EQ(testing::internal::GetCapturedStderr(), "");  // 標準エラー出力には出力されない

    ASSERT_EQ(handler.records.size(), 2U);
    EXPECT_EQ(handler.records[0].status, Error::Status::Warning);
    EXPECT_EQ(handler.records[0].type, Error::Type::IllegalCombination);
    EXPECT_EQ(handler.records[0].filename, "a.cpp");
    EXPECT_EQ(handler.records[0].funcname, "func_a");
    EXPECT_EQ(handler.records[0].line_number, 10U);
    EXPECT_EQ(handler.records[0].message, "warning 1");
    EXPECT_EQ(handler.records[1].status, Error::Status::Error);
    EXPECT_EQ(handler.records[1].message, "error x");

    // 状態も更新される
    EXPECT_EQ(error.get_status(), Error::Status::Error);
}

/// ErrorHandler が nullptr の場合は何も出力されないが、エラー状態は更新されることの確認
TEST_F(ErrorTest, NullHandlerIsSilent)
{
    Error& error = Error::get_instance();
    error.set_handler(nullptr);

    testing::internal::CaptureStderr();
    error.error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "silent");
    EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
    EXPECT_EQ(error.get_status(), Error::Status::Error);
}

/// 長いメッセージは、バッファの範囲内で切り捨てられる(溢れない)ことの確認
TEST_F(ErrorTest, LongMessageIsTruncated)
{
    RecordingErrorHandler handler;
    Error::get_instance().set_handler(&handler);

    const std::string long_message(1000, 'x');
    Error::get_instance().error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "%s", long_message.c_str());

    ASSERT_EQ(handler.records.size(), 1U);
    EXPECT_EQ(handler.records[0].message.size(), Error::max_message_length - 1);
}

/// ライブラリの設定関数が、エラーの内容(メッセージ)を ErrorHandler に渡すことの確認
TEST_F(ErrorTest, LibraryErrorsReachInjectedHandler)
{
    RecordingErrorHandler handler;
    Error::get_instance().set_handler(&handler);

    uint32_t id = 0;
    EXPECT_FALSE(can::get_motor_id(1, 1, 0, id));

    ASSERT_EQ(handler.records.size(), 1U);
    EXPECT_EQ(handler.records[0].status, Error::Status::Warning);
    EXPECT_NE(handler.records[0].message.find("out of range"), std::string::npos);
    EXPECT_NE(handler.records[0].filename.find("Id.cpp"), std::string::npos);
}

/// 複数のスレッドから通知、取得、リセットしても、データ競合しない(ThreadSanitizer で確認する)ことの確認
TEST_F(ErrorTest, ConcurrentUse)
{
    Error& error = Error::get_instance();
    error.set_handler(nullptr);

    std::atomic<bool>        stop{false};
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; i++) {
        threads.emplace_back([&]() {
            for (int n = 0; n < 20000; n++) {
                error.warning(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "w %d", n);
                error.error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "e %d", n);
            }
        });
    }
    threads.emplace_back([&]() {
        while (!stop.load()) {
            const Error::Status status = error.get_status();
            EXPECT_TRUE((status == Error::Status::Normal) || (status == Error::Status::Warning) ||
                        (status == Error::Status::Error));
            error.reset();
        }
    });

    threads[0].join();
    threads[1].join();
    stop.store(true);
    threads[2].join();
}

}  // namespace
