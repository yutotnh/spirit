// SPIRIT_ERROR_DEFAULT_HANDLER=0 でビルドしたときの Error のテスト
// 既定の ErrorHandler(標準エラー出力への出力)を含めないので、 ErrorHandler を設定しない限り、何も出力しない

#include <gtest/gtest.h>

#include "Error.h"
#include "tests/stubs.h"

#if SPIRIT_ERROR_DEFAULT_HANDLER
#error "This test must be built with SPIRIT_ERROR_DEFAULT_HANDLER=0"
#endif

namespace {

using namespace spirit;

TEST(ErrorNoDefaultHandler, NothingIsPrintedWithoutHandler)
{
    Error& error = Error::get_instance();
    EXPECT_EQ(Error::default_handler(), nullptr);
    EXPECT_EQ(error.get_handler(), nullptr);

    testing::internal::CaptureStderr();
    error.error(Error::Type::InvalidValue, __FILE__, __func__, __LINE__, "silent %d", 1);
    EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
    EXPECT_EQ(error.get_status(), Error::Status::Error);
}

TEST(ErrorNoDefaultHandler, InjectedHandlerStillWorks)
{
    RecordingErrorHandler handler;
    Error&                error = Error::get_instance();
    error.set_handler(&handler);
    error.warning(Error::Type::InvalidValue, "f.cpp", "func", 1, "message %d", 2);
    error.set_handler(nullptr);

    ASSERT_EQ(handler.records.size(), 1U);
    EXPECT_EQ(handler.records[0].message, "message 2");
}

}  // namespace
