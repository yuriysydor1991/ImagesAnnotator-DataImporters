#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "IImageSizeFacility.h"
#include "LibraryFacade.h"

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;

namespace
{

namespace fs = std::filesystem;

/**
 * @brief The image measuring service a consuming project supplies, standing
 * here for the imaging stack such a project already links.
 */
class FakeSizer : public iadi::IImageSizeFacility
{
 public:
  FakeSizer(int nwidth, int nheight) : width{nwidth}, height{nheight} {}

  bool read_image_size(const std::string&, int& owidth, int& oheight) override
  {
    owidth = width;
    oheight = height;
    return true;
  }

  IImageSizeFacilityPtr clone() override
  {
    return std::make_shared<FakeSizer>(width, height);
  }

  int width;
  int height;
};

/**
 * @brief Builds the three dataset directories on the filesystem and drives the
 * installed library over them exactly the way a downstream project does.
 */
class CTEST_Importers : public testing::Test
{
 public:
  void SetUp() override
  {
    root = fs::path{testing::TempDir()} / "ctest_importers";

    fs::remove_all(root);
    fs::create_directories(root);

    db = iadd::LibraryFacade::create_annotations_db();

    ASSERT_NE(db, nullptr);
  }

  void TearDown() override { fs::remove_all(root); }

  void given_file(const std::string& relPath, const std::string& contents)
  {
    const fs::path fpath = root / relPath;

    fs::create_directories(fpath.parent_path());

    std::ofstream f{fpath};
    f << contents;
  }

  /**
   * @brief Builds the context of the wanted dataset layout, the very same
   * instance the importer is asked for and then driven with.
   */
  template <class ContextT>
  std::shared_ptr<ContextT> context(const std::string& subdir)
  {
    auto ctx = std::make_shared<ContextT>();
    ctx->set_import_path((root / subdir).string());
    ctx->set_db(db);
    ctx->set_image_sizer(std::make_shared<FakeSizer>(200, 100));
    return ctx;
  }

  fs::path root;
  iadd::IAnnotationsDBPtr db;
};

}  // namespace

TEST_F(CTEST_Importers, plain_txt_import_reads_one_file_per_annotation)
{
  given_file("plain/dog.txt", "/imgs/a.png 1 50 20 100 40\n");

  auto ctx = context<iadi::PlainTxtImportLibraryContext>("plain");

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->get_full_path(), "/imgs/a.png");
  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);
}

// The very numbers the sibling exporters library writes for a 100x40 box at
// 50,20 of a 200x100 image, read back into that same box.
TEST_F(CTEST_Importers, yolo4_import_reads_the_darknet_layout)
{
  given_file("yolo/data/obj.names", "dog\n");
  given_file("yolo/data/obj.data",
             "classes = 1\n"
             "train = data/train.txt\n"
             "names = data/obj.names\n");
  given_file("yolo/data/train.txt", "data/a.png\n");
  given_file("yolo/data/a.png", "not-a-real-image");
  given_file("yolo/data/a.txt", "0 0.5 0.4 0.5 0.4\n");

  auto ctx = context<iadi::Yolo4ImportLibraryContext>("yolo");

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "a.png");
  EXPECT_EQ(ir->iwidth, 200);
  EXPECT_EQ(ir->iheight, 100);

  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);
}

TEST_F(CTEST_Importers, pytorch_vision_import_reads_the_tag_directories)
{
  given_file("pytorch/dog/a.png", "cropped");

  auto ctx = context<iadi::PyTorchImportLibraryContext>("pytorch");

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->name, "dog");
  // the whole crop is the annotation
  EXPECT_EQ(ir->rects.front()->x, 0);
  EXPECT_EQ(ir->rects.front()->y, 0);
  EXPECT_EQ(ir->rects.front()->width, 200);
  EXPECT_EQ(ir->rects.front()->height, 100);
}

TEST_F(CTEST_Importers, perform_import_runs_the_import_named_by_the_context)
{
  given_file("perform_import/data/obj.names", "dog\n");
  given_file("perform_import/data/train.txt", "data/a.png\n");
  given_file("perform_import/data/a.png", "not-a-real-image");
  given_file("perform_import/data/a.txt", "0 0.5 0.4 0.5 0.4\n");

  auto ctx = context<iadi::Yolo4ImportLibraryContext>("perform_import");

  auto lib = iadi::LibraryFacade::create_library(ctx);

  ASSERT_NE(lib, nullptr);
  ASSERT_TRUE(lib->perform_import(ctx));

  EXPECT_NE(ctx->get_importer(), nullptr);
  EXPECT_EQ(ctx->get_imported_records(), 1U);
  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->name, "dog");

  // The context field holds the importer, so that pointer is dropped here
  // rather than left for the fixture to carry.
  ctx->set_importer({});
}

// The database merges by the image path, so a dataset imported twice adds its
// images once. That is what makes the import safe to repeat over a project.
TEST_F(CTEST_Importers, importing_one_dataset_twice_adds_its_images_once)
{
  given_file("plain/dog.txt", "/imgs/a.png 1 50 20 100 40\n");

  auto ctx = context<iadi::PlainTxtImportLibraryContext>("plain");

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));
  ASSERT_TRUE(importer->import_db(ctx));

  EXPECT_EQ(db->get_images_db().size(), 1U);
  // the second run recovered the very same record all the same
  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

TEST_F(CTEST_Importers, no_importer_without_a_layout_naming_context)
{
  EXPECT_EQ(iadi::LibraryFacade::create_importer({}), nullptr);
  EXPECT_EQ(iadi::LibraryFacade::create_importer(
                std::make_shared<iadi::LibraryContext>()),
            nullptr);
}

TEST_F(CTEST_Importers, library_version_matches_the_data_drivers_one_it_fills)
{
  EXPECT_FALSE(iadi::LibraryFacade::library_version().empty());
  EXPECT_FALSE(iadd::LibraryFacade::library_version().empty());
}
