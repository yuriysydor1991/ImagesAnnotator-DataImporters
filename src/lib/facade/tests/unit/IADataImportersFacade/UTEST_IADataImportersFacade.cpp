#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include "IADataImportersFacade.h"
#include "src/lib/libmain/LibFactory.h"
#include "src/lib/libmain/LibMain.h"

using namespace ImagesAnnotatorDataImporters014;
using namespace iadi0impl;
using namespace testing;

class UTEST_IADataImportersFacade : public Test
{
 public:
  using LibFactory = iadi0impl::LibFactory;

  UTEST_IADataImportersFacade() = default;
  ~UTEST_IADataImportersFacade() = default;

  /**
   * @brief Drives one facade method and expects it to reach the factory.
   *
   * Every method of the facade is one and the same handful of lines - build
   * the factory, call the one method of it - so what a test of it states is
   * which method of that factory the called one reaches.
   *
   * @param expectation The factory method the call is to reach, set up on the
   * instance the facade builds.
   * @param call The facade method to drive.
   */
  void expect_reaches(const std::function<void(LibFactory&)>& expectation,
                      const std::function<void()>& call)
  {
    MockFunction<void(LibFactory&)> mockEnsurer;

    EXPECT_CALL(mockEnsurer, Call).Times(1).WillOnce(Invoke(expectation));

    LibFactory::onMockCreate = mockEnsurer.AsStdFunction();

    call();
  }
};

TEST_F(UTEST_IADataImportersFacade, create_library_reaches_the_appropriate_lib)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_appropriate_lib(_)).Times(1);
      },
      [] { IADataImportersFacade::create_library({}); });
}

TEST_F(UTEST_IADataImportersFacade, create_default_lib_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_default_lib()).Times(1);
      },
      [] { IADataImportersFacade::create_default_lib(); });
}

TEST_F(UTEST_IADataImportersFacade, create_plain_txt_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_plain_txt_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_plain_txt_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade, create_yolo4_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_yolo4_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_yolo4_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade,
       create_ultralytics_detect_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_ultralytics_detect_library_context())
            .Times(1);
      },
      [] {
        IADataImportersFacade::create_ultralytics_detect_library_context();
      });
}

TEST_F(UTEST_IADataImportersFacade,
       create_ultralytics_obb_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_ultralytics_obb_library_context())
            .Times(1);
      },
      [] { IADataImportersFacade::create_ultralytics_obb_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade,
       create_ultralytics_segment_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_ultralytics_segment_library_context())
            .Times(1);
      },
      [] {
        IADataImportersFacade::create_ultralytics_segment_library_context();
      });
}

TEST_F(UTEST_IADataImportersFacade, create_coco_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_coco_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_coco_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade, create_pascal_voc_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_pascal_voc_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_pascal_voc_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade, create_createml_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_createml_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_createml_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade, create_pytorch_library_context_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_pytorch_library_context()).Times(1);
      },
      [] { IADataImportersFacade::create_pytorch_library_context(); });
}

TEST_F(UTEST_IADataImportersFacade, create_importer_forwards_the_given_context)
{
  const IADataImportersContextPtr ctx =
      std::make_shared<IADataImportersContext>();

  expect_reaches(
      [ctx](LibFactory& instance) {
        EXPECT_CALL(instance, create_importer(ctx)).Times(1);
      },
      [ctx] { IADataImportersFacade::create_importer(ctx); });
}

TEST_F(UTEST_IADataImportersFacade, create_image_sizer_success)
{
  expect_reaches(
      [](LibFactory& instance) {
        EXPECT_CALL(instance, create_image_sizer()).Times(1);
      },
      [] { IADataImportersFacade::create_image_sizer(); });
}

TEST_F(UTEST_IADataImportersFacade, library_version_is_not_empty)
{
  LibFactory::onMockCreate = {};

  EXPECT_FALSE(IADataImportersFacade::library_version().empty());
}
