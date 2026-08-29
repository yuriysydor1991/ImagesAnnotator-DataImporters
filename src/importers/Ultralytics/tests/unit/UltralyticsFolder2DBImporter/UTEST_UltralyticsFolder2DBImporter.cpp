#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/Ultralytics/UltralyticsDetectFolder2DBImporter.h"
#include "src/importers/Ultralytics/UltralyticsObbFolder2DBImporter.h"
#include "src/importers/Ultralytics/UltralyticsSegmentFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::UltralyticsDetectFolder2DBImporter;
using iannotator::importers::UltralyticsObbFolder2DBImporter;
using iannotator::importers::UltralyticsSegmentFolder2DBImporter;

namespace iadd = ImagesAnnotatorDataDrivers012;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, so that
/// the un-normalisation is driven with numbers the test knows
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

class UTEST_UltralyticsFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_ultralytics_import";
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

  /// @brief Writes the descriptor the plain export produces, the path entry
  /// naming this very directory
  void given_the_descriptor()
  {
    given_file("data.yaml",
               "# The Ultralytics YOLO dataset descriptor, written by the "
               "ImagesAnnotator\n"
               "# annotations dataset exporters library.\n"
               "path: '" +
                   dir.string() +
                   "'\n"
                   "train: images/train\n"
                   "val: images/train\n"
                   "\n"
                   "names:\n"
                   "  0: 'cat'\n"
                   "  1: 'dog'\n");
  }

  /**
   * @brief Writes the directory of one image of 200x100 holding the one
   * rectangle the label line names.
   */
  void given_the_exported_dataset(const std::string& labelLine)
  {
    given_the_descriptor();
    given_file("images/train/street.png", "not-a-real-image");
    given_file("labels/train/street.txt", labelLine);
  }

  std::shared_ptr<IADataImportersContext> context(int width = 200,
                                                  int height = 100)
  {
    auto ctx = std::make_shared<IADataImportersContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    ctx->set_image_sizer(std::make_shared<FixedSizer>(width, height));
    return ctx;
  }

  /// @brief The one rectangle of the one imported record
  iadd::ImageRecordRectPtr the_rectangle()
  {
    if (db->get_images_db().size() != 1U ||
        db->get_images_db().front()->rects.size() != 1U) {
      return {};
    }

    return db->get_images_db().front()->rects.front();
  }
};

}  // namespace

// The very numbers the exporting side wrote for a 100x40 box at 50,20 of a
// 200x100 image, multiplied back by that very image.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_detection_layout_back)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");

  UltralyticsDetectFolder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "street.png");
  EXPECT_EQ(ir->abs_dir_path, (dir / "images" / "train").string());
  EXPECT_EQ(ir->iwidth, 200);
  EXPECT_EQ(ir->iheight, 100);

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

// The four corners of that very box, clockwise from the top left one.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_obb_layout_back)
{
  given_the_exported_dataset("1 0.25 0.2 0.75 0.2 0.75 0.6 0.25 0.6\n");

  UltralyticsObbFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

// The polygon outlining the object, which for a rectangle annotation is its
// own outline.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_segmentation_layout_back)
{
  given_the_exported_dataset("1 0.25 0.2 0.75 0.2 0.75 0.6 0.25 0.6\n");

  UltralyticsSegmentFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->width, 100);
}

// A polygon of any three or more points is what the format takes, and the
// rectangle which holds it is what the database keeps.
TEST_F(UTEST_UltralyticsFolder2DBImporter, holds_a_polygon_of_any_point_count)
{
  given_the_exported_dataset("0 0.25 0.2 0.75 0.4 0.5 0.6\n");

  UltralyticsSegmentFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "cat");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

TEST_F(UTEST_UltralyticsFolder2DBImporter,
       numbers_the_classes_the_descriptor_way)
{
  given_the_exported_dataset("0 0.5 0.4 0.5 0.4\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_NE(the_rectangle(), nullptr);
  EXPECT_EQ(the_rectangle()->name, "cat");
}

// The two list spellings of the names entry a dataset laid out by hand or by
// another tool arrives in.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_inline_names_list)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml",
             "train: images/train\n"
             "names: ['cat', \"dog\"]  # two classes\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_NE(the_rectangle(), nullptr);
  EXPECT_EQ(the_rectangle()->name, "dog");
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_block_names_list)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml",
             "names:\n"
             "  - cat\n"
             "  - dog\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_NE(the_rectangle(), nullptr);
  EXPECT_EQ(the_rectangle()->name, "dog");
}

// A name holding a colon or a hash stays one scalar, which is what the single
// quoted style of the export is for.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_a_quoted_name_whole)
{
  given_the_exported_dataset("0 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml",
             "train: images/train\n"
             "names:\n"
             "  0: 'a: b # c ''d'''\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_NE(the_rectangle(), nullptr);
  EXPECT_EQ(the_rectangle()->name, "a: b # c 'd'");
}

// Dropping the path line is what the exporting side documents as the way to
// move the directory, and a stale one names where it no longer is.
TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_a_directory_which_was_moved)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml",
             "path: '/nowhere/this/directory/is'\n"
             "train: images/train\n"
             "names:\n"
             "  0: 'cat'\n"
             "  1: 'dog'\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, reads_the_val_part_as_well)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml",
             "train: images/train\n"
             "val: images/val\n"
             "names:\n"
             "  1: 'dog'\n");
  given_file("images/val/park.jpg", "not-a-real-image");
  given_file("labels/val/park.txt", "1 0.5 0.4 0.5 0.4\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 2U);
}

// A training run reads an image without a label file as a picture holding
// none of the classes.
TEST_F(UTEST_UltralyticsFolder2DBImporter, keeps_an_image_without_a_label_file)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  fs::remove(dir / "labels" / "train" / "street.txt");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, skips_the_label_lines_it_cannot_read)
{
  given_the_exported_dataset(
      "1 0.5 0.4 0.5 0.4\n"
      "7 0.5 0.4 0.5 0.4\n"
      "1 0.5 0.4 0.5\n"
      "1 0.25 0.2 0.75 0.2 0.75 0.6 0.25 0.6\n"
      "1 0.5 0.4 0.0 0.4\n"
      "not a label line at all\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 1U);
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, refuses_a_polygon_of_two_points)
{
  given_the_exported_dataset("1 0.25 0.2 0.75 0.6\n");

  UltralyticsSegmentFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

// An Ultralytics release refuses a whole image over a coordinate outside the
// range, so this only ever reaches a dataset written by something else.
TEST_F(UTEST_UltralyticsFolder2DBImporter, cuts_a_box_reaching_over_the_edge)
{
  given_the_exported_dataset("1 0.5 0.5 2.0 2.0\n");

  UltralyticsDetectFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->x, 0);
  EXPECT_EQ(irr->y, 0);
  EXPECT_EQ(irr->width, 200);
  EXPECT_EQ(irr->height, 100);
}

// The boxes are normalised, so an image that stayed unmeasured carries no
// rectangle anybody could place.
TEST_F(UTEST_UltralyticsFolder2DBImporter, skips_an_image_it_cannot_measure)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FailingSizer>());

  UltralyticsDetectFolder2DBImporter importer;

  EXPECT_TRUE(importer.import_db(ctx));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, fails_without_a_descriptor)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  fs::remove(dir / "data.yaml");

  UltralyticsDetectFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_UltralyticsFolder2DBImporter,
       fails_over_a_descriptor_naming_no_class)
{
  given_the_exported_dataset("1 0.5 0.4 0.5 0.4\n");
  given_file("data.yaml", "train: images/train\n");

  UltralyticsDetectFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
}

TEST_F(UTEST_UltralyticsFolder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  UltralyticsDetectFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
