#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "IADataImportersContext.h"
#include "contexts/CocoImportContext.h"
#include "contexts/CreateMLImportContext.h"
#include "contexts/PascalVocImportContext.h"
#include "contexts/PlainTxtImportContext.h"
#include "contexts/PyTorchImportContext.h"
#include "contexts/UltralyticsDetectImportContext.h"
#include "contexts/UltralyticsObbImportContext.h"
#include "contexts/UltralyticsSegmentImportContext.h"
#include "contexts/Yolo4ImportContext.h"
#include "src/lib/libmain/LibFactory.h"

using namespace ImagesAnnotatorDataImporters014;
using namespace iadi0impl;
using namespace testing;

class UTEST_LibFactory : public Test
{
 public:
  UTEST_LibFactory() : factory{std::make_shared<LibFactory>()} {}

  std::shared_ptr<LibFactory> factory;
};

TEST_F(UTEST_LibFactory, create_default_lib_success)
{
  EXPECT_NE(factory->create_default_lib(), nullptr);
}

TEST_F(UTEST_LibFactory, create_library_context_of_every_layout_success)
{
  EXPECT_NE(factory->create_plain_txt_library_context(), nullptr);
  EXPECT_NE(factory->create_yolo4_library_context(), nullptr);
  EXPECT_NE(factory->create_ultralytics_detect_library_context(), nullptr);
  EXPECT_NE(factory->create_ultralytics_obb_library_context(), nullptr);
  EXPECT_NE(factory->create_ultralytics_segment_library_context(), nullptr);
  EXPECT_NE(factory->create_coco_library_context(), nullptr);
  EXPECT_NE(factory->create_pascal_voc_library_context(), nullptr);
  EXPECT_NE(factory->create_createml_library_context(), nullptr);
  EXPECT_NE(factory->create_pytorch_library_context(), nullptr);
}

// Every layout context has to name its own importer, which is the whole reason
// the factory hands out one method per layout instead of a default context.
TEST_F(UTEST_LibFactory, every_created_context_names_its_own_importer)
{
  EXPECT_NE(
      factory->create_importer(factory->create_plain_txt_library_context()),
      nullptr);
  EXPECT_NE(factory->create_importer(factory->create_yolo4_library_context()),
            nullptr);
  EXPECT_NE(factory->create_importer(
                factory->create_ultralytics_detect_library_context()),
            nullptr);
  EXPECT_NE(factory->create_importer(
                factory->create_ultralytics_obb_library_context()),
            nullptr);
  EXPECT_NE(factory->create_importer(
                factory->create_ultralytics_segment_library_context()),
            nullptr);
  EXPECT_NE(factory->create_importer(factory->create_coco_library_context()),
            nullptr);
  EXPECT_NE(
      factory->create_importer(factory->create_pascal_voc_library_context()),
      nullptr);
  EXPECT_NE(
      factory->create_importer(factory->create_createml_library_context()),
      nullptr);
  EXPECT_NE(factory->create_importer(factory->create_pytorch_library_context()),
            nullptr);
}

TEST_F(UTEST_LibFactory, create_appropriate_lib_success)
{
  EXPECT_NE(factory->create_appropriate_lib({}), nullptr);
}

TEST_F(UTEST_LibFactory, create_importer_gives_an_instance_for_every_context)
{
  EXPECT_NE(factory->create_importer(std::make_shared<PlainTxtImportContext>()),
            nullptr);
  EXPECT_NE(factory->create_importer(std::make_shared<Yolo4ImportContext>()),
            nullptr);
  EXPECT_NE(factory->create_importer(
                std::make_shared<UltralyticsDetectImportContext>()),
            nullptr);
  EXPECT_NE(
      factory->create_importer(std::make_shared<UltralyticsObbImportContext>()),
      nullptr);
  EXPECT_NE(factory->create_importer(
                std::make_shared<UltralyticsSegmentImportContext>()),
            nullptr);
  EXPECT_NE(factory->create_importer(std::make_shared<CocoImportContext>()),
            nullptr);
  EXPECT_NE(
      factory->create_importer(std::make_shared<PascalVocImportContext>()),
      nullptr);
  EXPECT_NE(factory->create_importer(std::make_shared<CreateMLImportContext>()),
            nullptr);
  EXPECT_NE(factory->create_importer(std::make_shared<PyTorchImportContext>()),
            nullptr);
}

TEST_F(UTEST_LibFactory, create_importer_without_a_layout_context_failure)
{
  EXPECT_EQ(factory->create_importer({}), nullptr);
  EXPECT_EQ(
      factory->create_importer(std::make_shared<IADataImportersContext>()),
      nullptr);
}

// The one image size reader case that runs in every configuration: with OpenCV
// the factory hands out the library's own reader, without it a nullptr, which
// is what makes IADataImportersContext::set_image_sizer() mandatory again. The
// reader itself is covered by UTEST_OpenCVImageSizer, configured only in an
// OpenCV build.
TEST_F(UTEST_LibFactory, create_image_sizer_matches_what_the_build_found)
{
  const auto sizer = factory->create_image_sizer();

#ifdef IADI_WITH_OPENCV
  EXPECT_NE(sizer, nullptr);
#else
  EXPECT_EQ(sizer, nullptr);
#endif  // IADI_WITH_OPENCV
}
