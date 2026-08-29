#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/Yolo4/Yolo4Folder2DBImporter.h"

using namespace testing;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::ImageRecordPtr;
using iannotator::importers::Yolo4Folder2DBImporter;

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

class UTEST_Yolo4Folder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_yolo4_import";
    fs::remove_all(dir);
    fs::create_directories(dir / "data");

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
   * @brief Writes the darknet directory the plain export produces: one class,
   * one image of 200x100 and the one rectangle of it.
   */
  void given_the_exported_dataset()
  {
    given_file("data/obj.names", "dog\ncat\n");
    given_file("data/obj.data",
               "classes = 2\n"
               "train = data/train.txt\n"
               "valid = data/val.txt\n"
               "names = data/obj.names\n"
               "backup = backup/\n");
    given_file("data/train.txt", "data/a.png\n");
    given_file("data/a.png", "not-a-real-image");
    given_file("data/a.txt", "0 0.5 0.4 0.5 0.4\n");
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
};

}  // namespace

// The very rectangle the exporting side divided by the image it was drawn in,
// multiplied back: a 100x40 box at 50,20 of a 200x100 image.
TEST_F(UTEST_Yolo4Folder2DBImporter, reads_the_darknet_layout_back)
{
  given_the_exported_dataset();

  Yolo4Folder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "a.png");
  EXPECT_EQ(ir->abs_dir_path, (dir / "data").string());
  EXPECT_EQ(ir->iwidth, 200);
  EXPECT_EQ(ir->iheight, 100);

  ASSERT_EQ(ir->rects.size(), 1U);

  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

TEST_F(UTEST_Yolo4Folder2DBImporter, numbers_the_classes_the_names_file_order)
{
  given_the_exported_dataset();
  given_file("data/a.txt", "1 0.5 0.4 0.5 0.4\n");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);

  EXPECT_EQ(db->get_images_db().front()->rects.front()->name, "cat");
}

// The descriptor is what a dataset laid out by some other tool is followed by,
// so the two lists are taken from wherever it names them.
TEST_F(UTEST_Yolo4Folder2DBImporter, follows_the_obj_data_descriptor)
{
  given_file("data/obj.names", "unused\n");
  given_file("names/classes.names", "dog\n");
  given_file("data/obj.data",
             "  names   =   names/classes.names  \n"
             "train = lists/images.txt\n");
  given_file("lists/images.txt", "pics/a.png\n");
  given_file("pics/a.png", "not-a-real-image");
  given_file("pics/a.txt", "0 0.5 0.5 1.0 1.0\n");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->abs_dir_path, (dir / "pics").string());
  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->name, "dog");
  // the whole image: a box of its size at its very origin
  EXPECT_EQ(ir->rects.front()->x, 0);
  EXPECT_EQ(ir->rects.front()->y, 0);
  EXPECT_EQ(ir->rects.front()->width, 200);
  EXPECT_EQ(ir->rects.front()->height, 100);
}

TEST_F(UTEST_Yolo4Folder2DBImporter, falls_back_to_the_val_list)
{
  given_the_exported_dataset();
  fs::remove(dir / "data" / "train.txt");
  given_file("data/val.txt", "data/a.png\n");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

TEST_F(UTEST_Yolo4Folder2DBImporter, falls_back_to_the_data_directory)
{
  given_the_exported_dataset();
  fs::remove(dir / "data" / "train.txt");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  // the descriptors of the layout are no images of it
  EXPECT_EQ(db->get_images_db().front()->path, "a.png");
}

TEST_F(UTEST_Yolo4Folder2DBImporter, skips_a_label_line_of_an_unknown_class)
{
  given_the_exported_dataset();
  given_file("data/a.txt",
             "0 0.5 0.4 0.5 0.4\n"
             "7 0.5 0.4 0.5 0.4\n"
             "not a label line at all\n");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->name, "dog");
}

// Darknet reads an image without a label file as one holding none of the
// classes, and such an image belongs to the project just as much.
TEST_F(UTEST_Yolo4Folder2DBImporter, keeps_an_image_without_a_label_file)
{
  given_the_exported_dataset();
  fs::remove(dir / "data" / "a.txt");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

TEST_F(UTEST_Yolo4Folder2DBImporter,
       skips_an_image_the_list_names_but_holds_not)
{
  given_the_exported_dataset();
  given_file("data/train.txt", "data/a.png\ndata/absent.png\n");

  Yolo4Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

// The boxes are normalised, so an image that stayed unmeasured carries no
// rectangle anybody could place.
TEST_F(UTEST_Yolo4Folder2DBImporter, skips_an_image_it_cannot_measure)
{
  given_the_exported_dataset();

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FailingSizer>());

  Yolo4Folder2DBImporter importer;

  EXPECT_TRUE(importer.import_db(ctx));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_Yolo4Folder2DBImporter, fails_without_the_class_names)
{
  given_the_exported_dataset();
  fs::remove(dir / "data" / "obj.names");

  Yolo4Folder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_Yolo4Folder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  Yolo4Folder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
