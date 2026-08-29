#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/PlainTxt/PlainTxtFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::ImageRecordPtr;
using iannotator::importers::PlainTxtFolder2DBImporter;

namespace iadd = ImagesAnnotatorDataDrivers012;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about
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

class UTEST_PlainTxtFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  // The real annotations database of the data drivers library, so the import
  // is seen through the very merge a consuming project performs.
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_plaintxt_import";
    fs::remove_all(dir);
    fs::create_directories(dir);

    db = iadd::IADataDriversFacade::create_annotations_db();

    ASSERT_NE(db, nullptr);
  }

  void TearDown() override { fs::remove_all(dir); }

  void given_tag_file(const std::string& name, const std::string& contents)
  {
    std::ofstream f{dir / name};
    f << contents;
  }

  std::shared_ptr<IADataImportersContext> context()
  {
    auto ctx = std::make_shared<IADataImportersContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    return ctx;
  }

  /// @brief The record of the given full image path, or a nullptr
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

TEST_F(UTEST_PlainTxtFolder2DBImporter, reads_one_annotation_per_txt_file)
{
  given_tag_file("cat.txt", "/imgs/a.png 2 1 2 3 4 5 6 7 8\n");
  given_tag_file("dog.txt", "/imgs/a.png 1 9 10 11 12\n");

  PlainTxtFolder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = record_of("/imgs/a.png");

  ASSERT_NE(ir, nullptr);

  // The record was named by two files, so it carries every rectangle of both
  ASSERT_EQ(ir->rects.size(), 3U);

  EXPECT_EQ(ir->rects[0]->name, "cat");
  EXPECT_EQ(ir->rects[0]->x, 1);
  EXPECT_EQ(ir->rects[0]->y, 2);
  EXPECT_EQ(ir->rects[0]->width, 3);
  EXPECT_EQ(ir->rects[0]->height, 4);

  EXPECT_EQ(ir->rects[1]->name, "cat");
  EXPECT_EQ(ir->rects[1]->x, 5);

  EXPECT_EQ(ir->rects[2]->name, "dog");
  EXPECT_EQ(ir->rects[2]->x, 9);
  EXPECT_EQ(ir->rects[2]->height, 12);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, splits_the_path_the_way_the_db_keeps_it)
{
  given_tag_file("cat.txt", "/imgs/sub/a.png 1 1 2 3 4\n");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  EXPECT_EQ(db->get_images_db().front()->abs_dir_path, "/imgs/sub");
  EXPECT_EQ(db->get_images_db().front()->path, "a.png");
}

// The export writes the path unquoted, so the count is what tells it from the
// rectangles behind it.
TEST_F(UTEST_PlainTxtFolder2DBImporter, reads_an_image_path_holding_a_space)
{
  given_tag_file("cat.txt", "/imgs/my cat.png 1 1 2 3 4\n");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  EXPECT_EQ(db->get_images_db().front()->get_full_path(), "/imgs/my cat.png");
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->name, "cat");
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, reads_the_negative_coordinates_back)
{
  given_tag_file("cat.txt", "/imgs/a.png 1 -5 -6 10 20\n");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);

  EXPECT_EQ(db->get_images_db().front()->rects.front()->x, -5);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->y, -6);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, skips_a_line_it_cannot_read)
{
  given_tag_file("cat.txt",
                 "/imgs/a.png 1 1 2 3 4\n"
                 "this line holds no count at all\n"
                 "/imgs/b.png 2 1 2 3 4\n"
                 "/imgs/c.png 1 1 2 3 4\n");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  // b.png promised two rectangles and wrote one, so its line is dropped whole
  ASSERT_EQ(db->get_images_db().size(), 2U);
  EXPECT_NE(record_of("/imgs/a.png"), nullptr);
  EXPECT_EQ(record_of("/imgs/b.png"), nullptr);
  EXPECT_NE(record_of("/imgs/c.png"), nullptr);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, ignores_everything_but_the_txt_files)
{
  given_tag_file("cat.txt", "/imgs/a.png 1 1 2 3 4\n");
  given_tag_file("notes.md", "/imgs/b.png 1 1 2 3 4\n");
  fs::create_directories(dir / "sub.txt");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_NE(record_of("/imgs/a.png"), nullptr);
}

// The layout carries no image size at all, so the records come out with the
// zeroes an unmeasured record holds - and every rectangle regardless, since
// those are stored in the image own pixels here.
TEST_F(UTEST_PlainTxtFolder2DBImporter,
       leaves_the_image_size_unset_without_a_sizer)
{
  given_tag_file("cat.txt", "/imgs/a.png 1 1 2 3 4\n");

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 0);
  EXPECT_EQ(db->get_images_db().front()->iheight, 0);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 1U);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, fills_the_image_size_in_when_it_can)
{
  given_tag_file("cat.txt", "/imgs/a.png 1 1 2 3 4\n");

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FixedSizer>(640, 480));

  PlainTxtFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 640);
  EXPECT_EQ(db->get_images_db().front()->iheight, 480);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, an_empty_directory_imports_nothing)
{
  PlainTxtFolder2DBImporter importer;

  auto ctx = context();

  EXPECT_TRUE(importer.import_db(ctx));
  EXPECT_TRUE(db->get_images_db().empty());
  EXPECT_EQ(ctx->get_imported_records(), 0U);
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, fails_when_import_path_is_empty)
{
  auto ctx = std::make_shared<IADataImportersContext>();
  ctx->set_db(db);

  PlainTxtFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, fails_without_a_database)
{
  auto ctx = std::make_shared<IADataImportersContext>();
  ctx->set_import_path(dir.string());

  PlainTxtFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}

TEST_F(UTEST_PlainTxtFolder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  PlainTxtFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
