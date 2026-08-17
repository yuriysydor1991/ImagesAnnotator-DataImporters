#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/PyTorch/PyTorchVisionFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::ImageRecordPtr;
using iannotator::importers::LibraryContext;
using iannotator::importers::PyTorchVisionFolder2DBImporter;

namespace iadd = ImagesAnnotatorDataDrivers011;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every crop asked about
class FixedSizer : public IImageSizeFacility
{
 public:
  FixedSizer(int nwidth, int nheight) : width{nwidth}, height{nheight} {}

  bool read_image_size(const std::string&, int& owidth, int& oheight) override
  {
    owidth = width;
    oheight = height;
    return true;
  }

  IImageSizeFacilityPtr clone() override
  {
    return std::make_shared<FixedSizer>(width, height);
  }

  int width;
  int height;
};

/// @brief Stands for a consumer whose imaging stack does not read the format
class FailingSizer : public IImageSizeFacility
{
 public:
  bool read_image_size(const std::string&, int&, int&) override
  {
    return false;
  }

  IImageSizeFacilityPtr clone() override
  {
    return std::make_shared<FailingSizer>();
  }
};

class UTEST_PyTorchVisionFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_pytorch_import";
    fs::remove_all(dir);
    fs::create_directories(dir);

    db = iadd::LibraryFacade::create_annotations_db();

    ASSERT_NE(db, nullptr);
  }

  void TearDown() override { fs::remove_all(dir); }

  void given_crop(const std::string& tagName, const std::string& fileName)
  {
    fs::create_directories(dir / tagName);

    std::ofstream f{dir / tagName / fileName};
    f << "cropped";
  }

  std::shared_ptr<LibraryContext> context(int width = 20, int height = 10)
  {
    auto ctx = std::make_shared<LibraryContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    ctx->set_image_sizer(std::make_shared<FixedSizer>(width, height));
    return ctx;
  }

  ImageRecordPtr record_of(const std::string& fullPath)
  {
    const auto& all = db->get_images_db();

    auto found = std::find_if(all.cbegin(), all.cend(),
                              [&fullPath](const ImageRecordPtr& ir) {
                                return ir->get_full_path() == fullPath;
                              });

    return found != all.cend() ? *found : ImageRecordPtr{};
  }
};

}  // namespace

// The crop is the annotation: one record per file, carrying the one rectangle
// over its whole area and named after the directory it was found in.
TEST_F(UTEST_PyTorchVisionFolder2DBImporter, one_record_per_crop_of_every_class)
{
  given_crop("dog", "a.png");
  given_crop("dog", "b.png");
  given_crop("cat", "c.png");

  PyTorchVisionFolder2DBImporter importer;

  auto ctx = context(20, 10);

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 3U);
  EXPECT_EQ(ctx->get_imported_records(), 3U);

  auto ir = record_of((dir / "dog" / "a.png").string());

  ASSERT_NE(ir, nullptr);

  EXPECT_EQ(ir->path, "a.png");
  EXPECT_EQ(ir->abs_dir_path, (dir / "dog").string());
  EXPECT_EQ(ir->iwidth, 20);
  EXPECT_EQ(ir->iheight, 10);

  ASSERT_EQ(ir->rects.size(), 1U);

  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 0);
  EXPECT_EQ(ir->rects.front()->y, 0);
  EXPECT_EQ(ir->rects.front()->width, 20);
  EXPECT_EQ(ir->rects.front()->height, 10);

  auto cat = record_of((dir / "cat" / "c.png").string());

  ASSERT_NE(cat, nullptr);
  ASSERT_EQ(cat->rects.size(), 1U);
  EXPECT_EQ(cat->rects.front()->name, "cat");
}

TEST_F(UTEST_PyTorchVisionFolder2DBImporter, ignores_a_file_beside_the_classes)
{
  given_crop("dog", "a.png");

  std::ofstream{dir / "README.md"} << "the classes are the directories";

  PyTorchVisionFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

TEST_F(UTEST_PyTorchVisionFolder2DBImporter, ignores_a_directory_below_a_class)
{
  given_crop("dog", "a.png");
  fs::create_directories(dir / "dog" / "nested");

  PyTorchVisionFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

// A crop of an unknown size is a rectangle of an unknown extent, which is no
// annotation at all.
TEST_F(UTEST_PyTorchVisionFolder2DBImporter, skips_a_crop_it_cannot_measure)
{
  given_crop("dog", "a.png");

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FailingSizer>());

  PyTorchVisionFolder2DBImporter importer;

  EXPECT_TRUE(importer.import_db(ctx));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_PyTorchVisionFolder2DBImporter, an_empty_directory_imports_nothing)
{
  PyTorchVisionFolder2DBImporter importer;

  auto ctx = context();

  EXPECT_TRUE(importer.import_db(ctx));
  EXPECT_TRUE(db->get_images_db().empty());
  EXPECT_EQ(ctx->get_imported_records(), 0U);
}

TEST_F(UTEST_PyTorchVisionFolder2DBImporter,
       fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  PyTorchVisionFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}

TEST_F(UTEST_PyTorchVisionFolder2DBImporter, fails_without_a_database)
{
  auto ctx = context();
  ctx->set_db({});

  PyTorchVisionFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
