#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "LibraryFacade.h"
#include "src/lib/libmain/LibFactory.h"
#include "src/lib/libmain/LibMain.h"

using namespace ImagesAnnotatorDataImporters011;
using namespace iadi0impl;
using namespace testing;

class UTEST_LibraryFacade : public Test
{
 public:
  using LibFactory = iadi0impl::LibFactory;

  UTEST_LibraryFacade() = default;
  ~UTEST_LibraryFacade() = default;
};

TEST_F(UTEST_LibraryFacade, create_default_lib_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_appropriate_lib(_)).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_library({});
}

TEST_F(UTEST_LibraryFacade, create_default_lib_direct_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_default_lib()).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_default_lib();
}

TEST_F(UTEST_LibraryFacade, create_plain_txt_library_context_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_plain_txt_library_context()).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_plain_txt_library_context();
}

TEST_F(UTEST_LibraryFacade, create_yolo4_library_context_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_yolo4_library_context()).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_yolo4_library_context();
}

TEST_F(UTEST_LibraryFacade, create_pytorch_library_context_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_pytorch_library_context()).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_pytorch_library_context();
}

TEST_F(UTEST_LibraryFacade, create_importer_forwards_the_given_context)
{
  MockFunction<void(LibFactory&)> mockEnsurer;
  const LibraryContextPtr ctx = std::make_shared<LibraryContext>();

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([ctx](LibFactory& instance) {
        EXPECT_CALL(instance, create_importer(ctx)).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_importer(ctx);
}

TEST_F(UTEST_LibraryFacade, create_image_sizer_success)
{
  MockFunction<void(LibFactory&)> mockEnsurer;

  EXPECT_CALL(mockEnsurer, Call)
      .Times(1)
      .WillOnce(Invoke([](LibFactory& instance) {
        EXPECT_CALL(instance, create_image_sizer()).Times(1);
      }));

  LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

  auto res = LibraryFacade::create_image_sizer();
}

TEST_F(UTEST_LibraryFacade, library_version_is_not_empty)
{
  LibFactory::onMockCreate = {};

  EXPECT_FALSE(LibraryFacade::library_version().empty());
}
