#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/PascalVoc/PascalVocFolder2DBImporter.h"

using namespace testing;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::LibraryContext;
using iannotator::importers::PascalVocFolder2DBImporter;

namespace iadd = ImagesAnnotatorDataDrivers011;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, which is
/// what a descriptor written without a size element is filled in from
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

class UTEST_PascalVocFolder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_pascalvoc_import";
    fs::remove_all(dir);
    fs::create_directories(dir);

    db = iadd::LibraryFacade::create_annotations_db();

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
   * @brief Writes the devkit directory the plain export produces: one image of
   * 640x400 under JPEGImages, its descriptor under Annotations and the lists
   * naming it.
   */
  void given_the_exported_dataset()
  {
    given_file("JPEGImages/street.png", "not-a-real-image");
    given_file("ImageSets/Main/train.txt", "street\n");
    given_file("ImageSets/Main/val.txt", "street\n");
    given_file("Annotations/street.xml",
               R"(<?xml version="1.0" encoding="UTF-8"?>
<annotation>
  <folder>JPEGImages</folder>
  <filename>street.png</filename>
  <source>
    <database>The ImagesAnnotator annotations dataset</database>
  </source>
  <size>
    <width>640</width>
    <height>400</height>
    <depth>3</depth>
  </size>
  <segmented>0</segmented>
  <object>
    <name>dog</name>
    <pose>Unspecified</pose>
    <truncated>0</truncated>
    <difficult>0</difficult>
    <bndbox>
      <xmin>50</xmin>
      <ymin>20</ymin>
      <xmax>150</xmax>
      <ymax>60</ymax>
    </bndbox>
  </object>
  <object>
    <name>cat &amp; friends</name>
    <pose>Unspecified</pose>
    <truncated>1</truncated>
    <difficult>1</difficult>
    <bndbox>
      <xmin>300</xmin>
      <ymin>25</ymin>
      <xmax>390</xmax>
      <ymax>70</ymax>
    </bndbox>
  </object>
</annotation>)");
  }

  std::shared_ptr<LibraryContext> context()
  {
    auto ctx = std::make_shared<LibraryContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    return ctx;
  }
};

}  // namespace

// The two corner points the rectangle was drawn between, turned back into the
// origin and the size an ImageRecordRect holds.
TEST_F(UTEST_PascalVocFolder2DBImporter, reads_the_exported_layout_back)
{
  given_the_exported_dataset();

  PascalVocFolder2DBImporter importer;

  auto ctx = context();

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->path, "street.png");
  EXPECT_EQ(ir->abs_dir_path, (dir / "JPEGImages").string());
  EXPECT_EQ(ir->iwidth, 640);
  EXPECT_EQ(ir->iheight, 400);

  ASSERT_EQ(ir->rects.size(), 2U);

  EXPECT_EQ(ir->rects.front()->name, "dog");
  EXPECT_EQ(ir->rects.front()->x, 50);
  EXPECT_EQ(ir->rects.front()->y, 20);
  EXPECT_EQ(ir->rects.front()->width, 100);
  EXPECT_EQ(ir->rects.front()->height, 40);

  // An annotation name is user text, and the export writes it XML escaped. A
  // difficult and truncated object is imported like every other one: a project
  // is edited and not evaluated.
  EXPECT_EQ(ir->rects.back()->name, "cat & friends");
  EXPECT_EQ(ir->rects.back()->width, 90);
  EXPECT_EQ(ir->rects.back()->height, 45);

  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

// Which is the shape LabelImg saves its own work in.
TEST_F(UTEST_PascalVocFolder2DBImporter,
       reads_the_descriptors_beside_the_images)
{
  given_file("a.png", "not-a-real-image");
  given_file("a.xml", R"(<annotation>
  <folder>images</folder>
  <filename>a.png</filename>
  <size><width>20</width><height>10</height><depth>3</depth></size>
  <object>
    <name>dog</name>
    <bndbox><xmin>1</xmin><ymin>2</ymin><xmax>5</xmax><ymax>8</ymax></bndbox>
  </object>
</annotation>)");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  EXPECT_EQ(ir->abs_dir_path, dir.string());
  ASSERT_EQ(ir->rects.size(), 1U);
  EXPECT_EQ(ir->rects.front()->x, 1);
  EXPECT_EQ(ir->rects.front()->width, 4);
  EXPECT_EQ(ir->rects.front()->height, 6);
}

TEST_F(UTEST_PascalVocFolder2DBImporter,
       reads_every_descriptor_of_the_directory)
{
  given_the_exported_dataset();
  given_file("JPEGImages/park.jpg", "not-a-real-image");
  given_file("Annotations/park.xml", R"(<annotation>
  <filename>park.jpg</filename>
  <size><width>20</width><height>10</height></size>
</annotation>)");

  // The lists name one part of the set each, and the whole set is imported
  // whichever of them names what.
  given_file("ImageSets/Main/train.txt", "street\n");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 2U);
}

TEST_F(UTEST_PascalVocFolder2DBImporter, keeps_an_image_carrying_no_object)
{
  given_file("JPEGImages/a.png", "not-a-real-image");
  given_file("Annotations/a.xml", R"(<annotation>
  <filename>a.png</filename>
  <size><width>20</width><height>10</height></size>
</annotation>)");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_TRUE(db->get_images_db().front()->rects.empty());
}

TEST_F(UTEST_PascalVocFolder2DBImporter, skips_the_objects_it_cannot_place)
{
  given_file("JPEGImages/a.png", "not-a-real-image");
  given_file("Annotations/a.xml", R"(<annotation>
  <filename>a.png</filename>
  <size><width>20</width><height>10</height></size>
  <object>
    <name>dog</name>
    <bndbox><xmin>1</xmin><ymin>2</ymin><xmax>5</xmax><ymax>8</ymax></bndbox>
  </object>
  <object>
    <bndbox><xmin>1</xmin><ymin>2</ymin><xmax>5</xmax><ymax>8</ymax></bndbox>
  </object>
  <object><name>cat</name></object>
  <object>
    <name>cat</name>
    <bndbox><xmin>1</xmin><ymin>2</ymin><xmax>1</xmax><ymax>8</ymax></bndbox>
  </object>
  <object>
    <name>cat</name>
    <bndbox><xmin>1</xmin><ymin>2</ymin><ymax>8</ymax></bndbox>
  </object>
</annotation>)");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 1U);
}

// The descriptor holds every rectangle of that image all the same, and a
// project whose pictures were moved is repaired by pointing it at them again.
TEST_F(UTEST_PascalVocFolder2DBImporter, keeps_an_image_the_directory_holds_not)
{
  given_the_exported_dataset();
  fs::remove(dir / "JPEGImages" / "street.png");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.size(), 2U);
  EXPECT_EQ(db->get_images_db().front()->abs_dir_path,
            (dir / "JPEGImages").string());
}

TEST_F(UTEST_PascalVocFolder2DBImporter,
       measures_an_image_described_without_a_size)
{
  given_file("JPEGImages/a.png", "not-a-real-image");
  given_file("Annotations/a.xml", R"(<annotation>
  <filename>a.png</filename>
  <object>
    <name>dog</name>
    <bndbox><xmin>1</xmin><ymin>2</ymin><xmax>5</xmax><ymax>8</ymax></bndbox>
  </object>
</annotation>)");

  auto ctx = context();
  ctx->set_image_sizer(std::make_shared<FixedSizer>(200, 100));

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 200);
  EXPECT_EQ(db->get_images_db().front()->iheight, 100);
}

TEST_F(UTEST_PascalVocFolder2DBImporter, skips_a_descriptor_of_another_kind)
{
  given_file("JPEGImages/a.png", "not-a-real-image");
  given_file("Annotations/broken.xml", "<annotation><filename>a.png");
  given_file("Annotations/other.xml", "<settings><a>1</a></settings>");
  given_file("Annotations/nameless.xml", "<annotation><size/></annotation>");
  given_file("Annotations/a.xml", R"(<annotation>
  <filename>a.png</filename>
  <size><width>20</width><height>10</height></size>
</annotation>)");

  PascalVocFolder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(db->get_images_db().size(), 1U);
}

TEST_F(UTEST_PascalVocFolder2DBImporter, fails_without_a_descriptor)
{
  given_file("JPEGImages/a.png", "not-a-real-image");

  PascalVocFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_PascalVocFolder2DBImporter, fails_on_a_missing_import_directory)
{
  auto ctx = context();
  ctx->set_import_path((dir / "absent").string());

  PascalVocFolder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(ctx));
}
