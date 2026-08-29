#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/Coco/CocoFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::CocoFolder2DBImporter;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;

namespace iadd = ImagesAnnotatorDataDrivers012;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, which is
/// what a descriptor written without one is filled in from
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

class UTEST_CocoFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_coco_import";
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
   * @brief Writes the directory the plain export produces: the descriptor of
   * two categories over one image of 640x400 and the two rectangles of it.
   */
  void given_the_exported_dataset()
  {
    given_file("images/street.png", "not-a-real-image");
    given_file("annotations/instances_default.json", R"({
"info": {"description": "The ImagesAnnotator annotations dataset"},
"licenses": [],
"images": [
  {"id": 1, "file_name": "street.png", "width": 640, "height": 400}
],
"annotations": [
  {"id": 1, "image_id": 1, "category_id": 2, "bbox": [50, 20, 100, 40],
   "area": 4000, "iscrowd": 0, "segmentation": []},
  {"id": 2, "image_id": 1, "category_id": 1, "bbox": [300, 25, 90, 45],
   "area": 4050, "iscrowd": 0, "segmentation": []}
],
"categories": [
  {"id": 1, "name": "cat", "supercategory": ""},
  {"id": 2, "name": "dog", "supercategory": ""}
]
})");
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

// The very four numbers the exporting side wrote, which are the ones an
// ImageRecordRect holds: nothing of a rectangle is computed back here.
TEST_F(UTEST_CocoFolder2DBImporter, reads_the_exported_layout_back)
{
  given_the_exported_dataset();

  CocoFolder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "street.png");
  EXPECT_EQ(ir->abs_dir_path, (dir / "images").string());
  EXPECT_EQ(ir->iwidth, 640);
  EXPECT_EQ(ir->iheight, 400);

  ASSERT_EQ(ir->rects.size(), 2U);

  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);

  EXPECT_EQ(ir->rects.back()->name, "cat");
  EXPECT_EQ(ir->rects.back()->x, 300);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

TEST_F(UTEST_CocoFolder2DBImporter, reads_a_box_written_with_a_fractional_part)
{
  given_file("images/a.png", "not-a-real-image");
  given_file("annotations/instances_default.json", R"({
"images": [{"id": 1, "file_name": "a.png", "width": 10, "height": 10}],
"annotations": [{"id": 1, "image_id": 1, "category_id": 1,
                 "bbox": [1.4, 1.5, 4.5, 4.4]}],
"categories": [{"id": 1, "name": "dog"}]})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);

  auto irr = db->get_images_db().front()->rects.front();

  EXPECT_EQ(irr->x, 1);
  EXPECT_EQ(irr->y, 2);
  EXPECT_EQ(irr->width, 5);
  EXPECT_EQ(irr->height, 4);
}

// The tools which hand out a single flat directory leave the descriptor
// beside the pictures it names.
TEST_F(UTEST_CocoFolder2DBImporter, reads_a_flat_directory)
{
  given_file("a.png", "not-a-real-image");
  given_file("_annotations.coco.json", R"({
"images": [{"id": 7, "file_name": "a.png", "width": 20, "height": 10}],
"annotations": [{"id": 1, "image_id": 7, "category_id": 3,
                 "bbox": [1, 2, 3, 4]}],
"categories": [{"id": 3, "name": "dog"}]})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->abs_dir_path, dir.string());
  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->name, "dog");
}

// The public releases of the format name their descriptor after the very
// directory the pictures of it lie in.
TEST_F(UTEST_CocoFolder2DBImporter, reads_the_directory_the_descriptor_names)
{
  given_file("train2017/a.png", "not-a-real-image");
  given_file("annotations/instances_train2017.json", R"({
"images": [{"id": 1, "file_name": "a.png", "width": 20, "height": 10}],
"annotations": [],
"categories": []})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->abs_dir_path,
            (dir / "train2017").string());
}

TEST_F(UTEST_CocoFolder2DBImporter, keeps_an_image_carrying_no_annotation)
{
  given_the_exported_dataset();
  given_file("images/park.jpg", "not-a-real-image");
  given_file("annotations/instances_default.json", R"({
"images": [{"id": 1, "file_name": "park.jpg", "width": 20, "height": 10}],
"annotations": [],
"categories": [{"id": 1, "name": "dog"}]})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

// The descriptor holds every rectangle of that image all the same, and a
// project whose pictures were moved is repaired by pointing it at them again.
TEST_F(UTEST_CocoFolder2DBImporter, keeps_an_image_the_directory_holds_not)
{
  given_the_exported_dataset();
  fs::remove(dir / "images" / "street.png");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 2U);
}

TEST_F(UTEST_CocoFolder2DBImporter, skips_the_annotations_it_cannot_place)
{
  given_file("images/a.png", "not-a-real-image");
  given_file("annotations/instances_default.json", R"({
"images": [{"id": 1, "file_name": "a.png", "width": 20, "height": 10}],
"annotations": [
  {"id": 1, "image_id": 1, "category_id": 1, "bbox": [1, 2, 3, 4]},
  {"id": 2, "image_id": 9, "category_id": 1, "bbox": [1, 2, 3, 4]},
  {"id": 3, "image_id": 1, "category_id": 9, "bbox": [1, 2, 3, 4]},
  {"id": 4, "image_id": 1, "category_id": 1, "bbox": [1, 2, 3]},
  {"id": 5, "image_id": 1, "category_id": 1, "bbox": [1, 2, 0, 4]},
  {"id": 6, "image_id": 1, "category_id": 1}
],
"categories": [{"id": 1, "name": "dog"}]})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 1U);
}

// Which is no descriptor the sibling exporters library ever wrote, since the
// format carries the size of every image it names.
TEST_F(UTEST_CocoFolder2DBImporter, measures_an_image_declared_without_a_size)
{
  given_file("images/a.png", "not-a-real-image");
  given_file("annotations/instances_default.json", R"({
"images": [{"id": 1, "file_name": "a.png"}],
"annotations": [],
"categories": []})");

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FixedSizer>(200, 100));

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 200);
  EXPECT_EQ(db->get_images_db().front()->iheight, 100);
}

TEST_F(UTEST_CocoFolder2DBImporter, fails_without_a_descriptor)
{
  given_file("images/a.png", "not-a-real-image");

  CocoFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

// A JSON file which is no descriptor of this layout is no descriptor of it.
TEST_F(UTEST_CocoFolder2DBImporter, fails_over_a_json_file_of_another_kind)
{
  given_file("annotations/instances_default.json", R"({"a": 1})");

  CocoFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
}

TEST_F(UTEST_CocoFolder2DBImporter, reads_the_descriptor_beside_a_broken_one)
{
  given_file("images/a.png", "not-a-real-image");
  given_file("annotations/broken.json", "{not a json document");
  given_file("annotations/instances.json", R"({
"images": [{"id": 1, "file_name": "a.png", "width": 20, "height": 10}],
"annotations": [],
"categories": []})");

  CocoFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

TEST_F(UTEST_CocoFolder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  CocoFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
