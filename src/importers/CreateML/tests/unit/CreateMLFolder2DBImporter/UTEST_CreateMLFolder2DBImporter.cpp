#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/CreateML/CreateMLFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::CreateMLFolder2DBImporter;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;

namespace iadd = ImagesAnnotatorDataDrivers012;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, since
/// this layout carries no size of its own anywhere
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

class UTEST_CreateMLFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_createml_import";
    fs::remove_all(dir);
    fs::create_directories(dir);

    db = iadd::IADataDriversFacade::create_annotations_db();

    ASSERT_NE(db, nullptr);
  }

  void TearDown() override { fs::remove_all(dir); }

  void given_file(const std::string& relPath, const std::string& contents)
  {
    const fs::path fpath = dir / relPath;

    fs::create_directories(fpath.parent_path());

    std::ofstream f{fpath};
    f << contents;
  }

  /**
   * @brief Writes the flat directory the plain export produces: the descriptor
   * of one image and the two rectangles drawn over it, one of them of an odd
   * size and therefore centred on a half pixel.
   */
  void given_the_exported_dataset()
  {
    given_file("street.png", "not-a-real-image");
    given_file("annotations.json", R"([
  {"imagefilename": "street.png", "annotation": [
    {"label": "dog", "coordinates": {"x": 100, "y": 40, "width": 100,
                                     "height": 40}},
    {"label": "cat", "coordinates": {"x": 345, "y": 47.5, "width": 90,
                                     "height": 45}}
  ]}
])");
  }

  std::shared_ptr<IADataImportersContext> context()
  {
    auto ctx = std::make_shared<IADataImportersContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    return ctx;
  }
};

}  // namespace

// The very halving the exporting side performed, undone: a 100x40 box drawn
// at 50,20 is written centred on 100,40 and comes back where it was.
TEST_F(UTEST_CreateMLFolder2DBImporter, reads_the_exported_layout_back)
{
  given_the_exported_dataset();

  CreateMLFolder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "street.png");
  EXPECT_EQ(ir->abs_dir_path, dir.string());

  ASSERT_EQ(ir->rects.size(), 2U);

  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);

  // The half pixel of an odd sized box lands back on the very pixel it was
  // drawn from.
  EXPECT_EQ(ir->rects.back()->name, "cat");
  EXPECT_EQ(ir->rects.back()->x, 300);
  EXPECT_EQ(ir->rects.back()->y, 25);
  EXPECT_EQ(ir->rects.back()->width, 90);
  EXPECT_EQ(ir->rects.back()->height, 45);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

// The plural spelling of the two keys is the dialect of the format some
// converters emit.
TEST_F(UTEST_CreateMLFolder2DBImporter, reads_the_plural_spelling_of_the_keys)
{
  given_file("a.png", "not-a-real-image");
  given_file("annotations.json", R"([
  {"image": "a.png", "annotations": [
    {"label": "dog", "coordinates": {"x": 10, "y": 10, "width": 4,
                                     "height": 4}}
  ]}
])");

  CreateMLFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->x, 8);
}

TEST_F(UTEST_CreateMLFolder2DBImporter, keeps_an_image_carrying_no_annotation)
{
  given_file("a.png", "not-a-real-image");
  given_file("annotations.json",
             R"([{"imagefilename": "a.png", "annotation": []}])");

  CreateMLFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

TEST_F(UTEST_CreateMLFolder2DBImporter, skips_the_annotations_it_cannot_place)
{
  given_file("a.png", "not-a-real-image");
  given_file("annotations.json", R"([
  {"imagefilename": "a.png", "annotation": [
    {"label": "dog", "coordinates": {"x": 10, "y": 10, "width": 4,
                                     "height": 4}},
    {"coordinates": {"x": 10, "y": 10, "width": 4, "height": 4}},
    {"label": "", "coordinates": {"x": 10, "y": 10, "width": 4, "height": 4}},
    {"label": "cat"},
    {"label": "cat", "coordinates": {"x": 10, "y": 10, "width": 0,
                                     "height": 4}},
    {"label": "cat", "coordinates": {"x": 10, "width": 4, "height": 4}}
  ]},
  {"annotation": []}
])");

  CreateMLFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 1U);
}

// The layout carries no size anywhere, so a record is complete without one -
// and carries it when the consumer supplied a way to measure the picture.
TEST_F(UTEST_CreateMLFolder2DBImporter, measures_the_images_when_it_can)
{
  given_the_exported_dataset();

  CreateMLFolder2DBImporter importer;

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FixedSizer>(640, 400));

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 640);
  EXPECT_EQ(db->get_images_db().front()->iheight, 400);
}

// The name is reserved by the export, so an image record really named that
// way was written out under another one - and the descriptor with it.
TEST_F(UTEST_CreateMLFolder2DBImporter, reads_a_descriptor_of_another_name)
{
  given_file("annotations.json", "not-a-real-image");
  given_file("a.png", "not-a-real-image");
  given_file("annotations-1.json", R"([
  {"imagefilename": "a.png", "annotation": [
    {"label": "dog", "coordinates": {"x": 10, "y": 10, "width": 4,
                                     "height": 4}}
  ]}
])");

  CreateMLFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->path, "a.png");
}

TEST_F(UTEST_CreateMLFolder2DBImporter, fails_without_a_descriptor)
{
  given_file("a.png", "not-a-real-image");

  CreateMLFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

// An array of anything else is no descriptor of this layout.
TEST_F(UTEST_CreateMLFolder2DBImporter, fails_over_a_json_file_of_another_kind)
{
  given_file("annotations.json", R"([{"a": 1}])");

  CreateMLFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
}

TEST_F(UTEST_CreateMLFolder2DBImporter, reads_a_descriptor_of_no_image_at_all)
{
  given_file("annotations.json", "[]");

  CreateMLFolder2DBImporter importer;

  EXPECT_TRUE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_CreateMLFolder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  CreateMLFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
