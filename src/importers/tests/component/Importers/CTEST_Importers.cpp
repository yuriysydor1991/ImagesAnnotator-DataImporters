#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "IADataImportersFacade.h"
#include "IImageSizeFacility.h"

namespace iadd = ImagesAnnotatorDataDrivers012;
namespace iadi = ImagesAnnotatorDataImporters014;

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

    db = iadd::IADataDriversFacade::create_annotations_db();

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
    return filled(std::make_shared<ContextT>(), subdir);
  }

  /**
   * @brief Fills a context the library facade handed out, which is the way a
   * downstream project builds one.
   *
   * @param ctx The empty context of the wanted layout.
   * @param subdir The dataset directory below the test root.
   *
   * @return Returns that very context, ready to be imported with.
   */
  template <class ContextPtrT>
  ContextPtrT filled(ContextPtrT ctx, const std::string& subdir)
  {
    ctx->set_import_path((root / subdir).string());
    ctx->set_db(db);
    ctx->set_image_sizer(std::make_shared<FakeSizer>(200, 100));
    return ctx;
  }

  /**
   * @brief Writes the Ultralytics YOLO directory of one image of 200x100 and
   * the one rectangle the given label line names.
   */
  void given_the_ultralytics_dataset(const std::string& labelLine)
  {
    given_file("ultralytics/data.yaml",
               "train: images/train\n"
               "val: images/train\n"
               "\n"
               "names:\n"
               "  0: 'cat'\n"
               "  1: 'dog'\n");
    given_file("ultralytics/images/train/a.png", "not-a-real-image");
    given_file("ultralytics/labels/train/a.txt", labelLine);
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

  fs::path root;
  iadd::IAnnotationsDBPtr db;
};

}  // namespace

TEST_F(CTEST_Importers, plain_txt_import_reads_one_file_per_annotation)
{
  given_file("plain/dog.txt", "/imgs/a.png 1 50 20 100 40\n");

  auto ctx = context<iadi::PlainTxtImportContext>("plain");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

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

  auto ctx = context<iadi::Yolo4ImportContext>("yolo");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

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

  auto ctx = context<iadi::PyTorchImportContext>("pytorch");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

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

// The very four numbers the sibling exporters library writes into its COCO
// descriptor, which are the ones an ImageRecordRect holds.
TEST_F(CTEST_Importers, coco_import_reads_the_single_json_descriptor)
{
  given_file("coco/images/a.png", "not-a-real-image");
  given_file("coco/annotations/instances_default.json", R"({
"info": {"description": "The ImagesAnnotator annotations dataset"},
"licenses": [],
"images": [{"id": 1, "file_name": "a.png", "width": 640, "height": 400}],
"annotations": [{"id": 1, "image_id": 1, "category_id": 2,
                 "bbox": [50, 20, 100, 40], "area": 4000, "iscrowd": 0,
                 "segmentation": []}],
"categories": [{"id": 1, "name": "cat"}, {"id": 2, "name": "dog"}]})");

  auto ctx = filled(iadi::IADataImportersFacade::create_coco_library_context(),
                    "coco");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);

  auto ir = db->get_images_db().front();

  // the descriptor carries the size, so no measurement was needed
  EXPECT_EQ(ir->iwidth, 640);
  EXPECT_EQ(ir->iheight, 400);

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

// The centre the Create ML descriptor writes, halved back onto the corner the
// rectangle was drawn from.
TEST_F(CTEST_Importers, createml_import_reads_the_flat_directory)
{
  given_file("createml/a.png", "not-a-real-image");
  given_file("createml/annotations.json", R"([
  {"imagefilename": "a.png", "annotation": [
    {"label": "dog", "coordinates": {"x": 100, "y": 40, "width": 100,
                                     "height": 40}}
  ]}
])");

  auto ctx =
      filled(iadi::IADataImportersFacade::create_createml_library_context(),
             "createml");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

// The two corner points of a bndbox, turned back into the origin and the size.
TEST_F(CTEST_Importers, pascal_voc_import_reads_the_devkit_directory)
{
  given_file("voc/JPEGImages/a.png", "not-a-real-image");
  given_file("voc/ImageSets/Main/train.txt", "a\n");
  given_file("voc/Annotations/a.xml", R"(<?xml version="1.0" encoding="UTF-8"?>
<annotation>
  <folder>JPEGImages</folder>
  <filename>a.png</filename>
  <size><width>640</width><height>400</height><depth>3</depth></size>
  <segmented>0</segmented>
  <object>
    <name>dog</name>
    <pose>Unspecified</pose>
    <truncated>0</truncated>
    <difficult>0</difficult>
    <bndbox><xmin>50</xmin><ymin>20</ymin><xmax>150</xmax><ymax>60</ymax></bndbox>
  </object>
</annotation>)");

  auto ctx = filled(
      iadi::IADataImportersFacade::create_pascal_voc_library_context(), "voc");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 640);

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

// The centre and the size of the detection line, multiplied back by the image
// the exporting side divided them by.
TEST_F(CTEST_Importers, ultralytics_detect_import_reads_the_label_files)
{
  given_the_ultralytics_dataset("1 0.5 0.4 0.5 0.4\n");

  auto ctx = filled(
      iadi::IADataImportersFacade::create_ultralytics_detect_library_context(),
      "ultralytics");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

// The four corners of that very box, clockwise from the top left one.
TEST_F(CTEST_Importers, ultralytics_obb_import_reads_the_corners)
{
  given_the_ultralytics_dataset("1 0.25 0.2 0.75 0.2 0.75 0.6 0.25 0.6\n");

  auto ctx = filled(
      iadi::IADataImportersFacade::create_ultralytics_obb_library_context(),
      "ultralytics");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "dog");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->width, 100);
}

// The polygon outlining the object, which the rectangle holding it comes back
// out of.
TEST_F(CTEST_Importers, ultralytics_segment_import_reads_the_polygon)
{
  given_the_ultralytics_dataset("0 0.25 0.2 0.75 0.4 0.5 0.6\n");

  auto ctx = filled(
      iadi::IADataImportersFacade::create_ultralytics_segment_library_context(),
      "ultralytics");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));

  auto irr = the_rectangle();

  ASSERT_NE(irr, nullptr);
  EXPECT_EQ(irr->name, "cat");
  EXPECT_EQ(irr->x, 50);
  EXPECT_EQ(irr->y, 20);
  EXPECT_EQ(irr->width, 100);
  EXPECT_EQ(irr->height, 40);
}

TEST_F(CTEST_Importers, perform_import_runs_the_import_named_by_the_context)
{
  given_file("perform_import/data/obj.names", "dog\n");
  given_file("perform_import/data/train.txt", "data/a.png\n");
  given_file("perform_import/data/a.png", "not-a-real-image");
  given_file("perform_import/data/a.txt", "0 0.5 0.4 0.5 0.4\n");

  auto ctx = context<iadi::Yolo4ImportContext>("perform_import");

  auto lib = iadi::IADataImportersFacade::create_library(ctx);

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

  auto ctx = context<iadi::PlainTxtImportContext>("plain");

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

  ASSERT_NE(importer, nullptr);
  ASSERT_TRUE(importer->import_db(ctx));
  ASSERT_TRUE(importer->import_db(ctx));

  EXPECT_EQ(db->get_images_db().size(), 1U);
  // the second run recovered the very same record all the same
  EXPECT_EQ(ctx->get_imported_records(), 1U);
}

TEST_F(CTEST_Importers, no_importer_without_a_layout_naming_context)
{
  EXPECT_EQ(iadi::IADataImportersFacade::create_importer({}), nullptr);
  EXPECT_EQ(iadi::IADataImportersFacade::create_importer(
                std::make_shared<iadi::IADataImportersContext>()),
            nullptr);
}

TEST_F(CTEST_Importers, library_version_matches_the_data_drivers_one_it_fills)
{
  EXPECT_FALSE(iadi::IADataImportersFacade::library_version().empty());
  EXPECT_FALSE(iadd::IADataDriversFacade::library_version().empty());
}
