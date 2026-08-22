#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "IImporter.h"
#include "LibraryContext.h"
#include "src/lib/libmain/LibFactory.h"
#include "src/lib/libmain/LibMain.h"

using namespace ImagesAnnotatorDataImporters013;
using namespace iadi0impl;
using namespace testing;

namespace
{

class ImporterMock : public IImporter
{
 public:
  MOCK_METHOD(bool, import_db, (LibraryContextPtr ctx), (override));
};

}  // namespace

class UTEST_LibMain : public Test
{
 public:
  UTEST_LibMain() : libmain{std::make_shared<LibMain>()} {}

  ~UTEST_LibMain() { LibFactory::onMockCreate = {}; }

  LibraryContextPtr filled_context()
  {
    auto ctx = std::make_shared<LibraryContext>();
    ctx->set_import_path("/tmp/some-import-dir");
    return ctx;
  }

  std::shared_ptr<LibMain> libmain;
};

TEST_F(UTEST_LibMain, perform_import_no_context_failure)
{
  LibFactory::onMockCreate = {};

  EXPECT_FALSE(libmain->perform_import({}));
}

TEST_F(UTEST_LibMain, perform_import_no_importer_for_the_context_failure)
{
  LibFactory::onMockCreate = [](LibFactory& instance) {
    EXPECT_CALL(instance, create_importer(_))
        .Times(1)
        .WillOnce(Return(IImporterPtr{}));
  };

  EXPECT_FALSE(libmain->perform_import(filled_context()));
}

TEST_F(UTEST_LibMain, perform_import_reports_the_importer_failure)
{
  auto importer = std::make_shared<ImporterMock>();
  auto ctx = filled_context();

  EXPECT_CALL(*importer, import_db(_)).Times(1).WillOnce(Return(false));

  LibFactory::onMockCreate = [importer, ctx](LibFactory& instance) {
    EXPECT_CALL(instance, create_importer(ctx))
        .Times(1)
        .WillOnce(Return(importer));
  };

  EXPECT_FALSE(libmain->perform_import(ctx));
}

TEST_F(UTEST_LibMain,
       perform_import_success_publishes_the_importer_and_passes_the_context)
{
  auto importer = std::make_shared<ImporterMock>();
  auto ctx = filled_context();

  EXPECT_CALL(*importer, import_db(ctx)).Times(1).WillOnce(Return(true));

  LibFactory::onMockCreate = [importer, ctx](LibFactory& instance) {
    EXPECT_CALL(instance, create_importer(ctx))
        .Times(1)
        .WillOnce(Return(importer));
  };

  EXPECT_TRUE(libmain->perform_import(ctx));

  EXPECT_EQ(ctx->get_importer(), importer);

  // The published field holds the importer and the expectation above holds
  // the context, so that pointer cycle has to be broken for the mock to be
  // freed and verified at all.
  ctx->set_importer({});
}
