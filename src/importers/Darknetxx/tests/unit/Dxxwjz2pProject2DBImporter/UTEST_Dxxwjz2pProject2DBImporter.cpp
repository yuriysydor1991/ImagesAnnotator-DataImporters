#include <ImagesAnnotatorDataDrivers-0.13/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

#include "src/importers/Darknetxx/Dxxwjz2pProject2DBImporter.h"
#include "src/importers/Darknetxx/Dxxwjz3pProject2DBImporter.h"
#include "src/zlib/ZlibController.h"

using namespace testing;
using iannotator::importers::Dxxwjz2pProject2DBImporter;
using iannotator::importers::Dxxwjz3pProject2DBImporter;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;
using iannotator::importers::IImporter;

namespace iadd = ImagesAnnotatorDataDrivers013;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, which is
/// what an image of no size in the project is filled in from
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

/// @brief The annotations member the sibling exporters library writes: the
/// one folder of the copied images, of two images and three boxes
const std::string annotations = R"(
  "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
    {"db": {"path": "images"}, "annotations": [
      {"rel_path": "park.jpg", "width": 640, "height": 480, "annotations": [{"name": "dog", "rect": {"x": 200, "y": 130, "width": 48, "height": 52}}, {"name": "cat", "rect": {"x": 10, "y": 20, "width": 30, "height": 40}}]},
      {"rel_path": "street.png", "width": 320, "height": 200, "validation": true, "annotations": [{"name": "cat", "rect": {"x": 1, "y": 2, "width": 3, "height": 4}}]}
    ]}
  ]})";

std::string project_of(const std::string& format, const std::string& network,
                       const std::string& member = annotations)
{
  return R"({
  "format": ")" +
         format + R"(",
  "version": 1,
  "generator": "darknetxx 0.11.0",
  "network": )" +
         network + R"(,
  "training": {"seconds": 0,
    "loss": [],
    "avg_loss": []
  },)" + member +
         R"(
})";
}

const std::string dxxwjz2Network =
    R"({"cfg": "[net]\nbatch=64\n", "classes": [
    {"index": 0, "name": "cat"},
    {"index": 1, "name": "dog"}
  ]})";

/// @brief The network of a trained dxxwjz3p project: the weights of the
/// layers come before the annotations
const std::string dxxwjz3Network =
    R"({"net": {"batch": 64}, "layers": [
    {"type": "convolutional", "options": {"filters": 32}, "arrays": {"biases": [0.5,-0.25,"0x7fc00000"], "weights": [0.125,1e-45]}},
    {"type": "yolo", "options": {"mask": [0,1,2], "classes": 2}}
  ], "classes": ["cat", "dog"]})";

class UTEST_Dxxwjz2pProject2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_dxxwjz2p_import";
    fs::remove_all(dir);
    fs::create_directories(dir / "images");

    db = iadd::IADataDriversFacade::create_annotations_db();

    ASSERT_NE(db, nullptr);
  }

  void TearDown() override { fs::remove_all(dir); }

  void given_file(const fs::path& fpath, const std::string& contents)
  {
    fs::create_directories(fpath.parent_path());

    std::ofstream f{fpath, std::ios::binary};
    f << contents;
  }

  /// @brief Writes the document gzip compressed, the way the darknetxx does
  void given_gzip(const fs::path& fpath, const std::string& document)
  {
    fs::create_directories(fpath.parent_path());

    ASSERT_TRUE(iadi0zlibi::ZlibController::create()->compress_to_file(
        fpath.string(), {document.cbegin(), document.cend()}));
  }

  void given_the_images()
  {
    given_file(dir / "images/park.jpg", "not-a-real-image");
    given_file(dir / "images/street.png", "not-a-real-image");
  }

  std::shared_ptr<IADataImportersContext> context(const fs::path& path)
  {
    auto ctx = std::make_shared<IADataImportersContext>();
    ctx->set_import_path(path.string());
    ctx->set_db(db);
    return ctx;
  }

  void expect_the_exported_records()
  {
    ASSERT_EQ(db->get_images_db().size(), 2U);

    const auto& records = db->get_images_db();

    auto park =
        records.front()->path == "park.jpg" ? records.front() : records.back();
    auto street =
        records.front()->path == "park.jpg" ? records.back() : records.front();

    EXPECT_EQ(park->path, "park.jpg");
    EXPECT_EQ(park->abs_dir_path, (dir / "images").string());
    EXPECT_EQ(park->iwidth, 640);
    EXPECT_EQ(park->iheight, 480);

    ASSERT_EQ(park->rects.size(), 2U);

    EXPECT_EQ(park->rects[0]->name, "dog");
    EXPECT_EQ(park->rects[0]->x, 200);
    EXPECT_EQ(park->rects[0]->y, 130);
    EXPECT_EQ(park->rects[0]->width, 48);
    EXPECT_EQ(park->rects[0]->height, 52);
    EXPECT_EQ(park->rects[1]->name, "cat");

    EXPECT_EQ(street->path, "street.png");
    EXPECT_EQ(street->iwidth, 320);
    ASSERT_EQ(street->rects.size(), 1U);
    EXPECT_EQ(street->rects[0]->width, 3);
  }
};

}  // namespace

// The directory the sibling exporters library writes: the gzip compressed
// project beside the folder of the copied images, the import path naming the
// directory.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, reads_the_exported_project_back)
{
  given_the_images();
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz2pProject2DBImporter importer;

  auto ctx = context(dir);

  ASSERT_TRUE(importer.import_db(ctx));

  expect_the_exported_records();

  EXPECT_EQ(ctx->get_imported_records(), 2U);
}

TEST_F(UTEST_Dxxwjz2pProject2DBImporter, reads_the_project_file_of_the_path)
{
  given_the_images();
  given_gzip(dir / "trained.dxxwjz2p", project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir / "trained.dxxwjz2p")));

  expect_the_exported_records();
}

// The one file of another name of the directory is the project as well.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, finds_the_project_of_another_name)
{
  given_the_images();
  given_gzip(dir / "trained.dxxwjz2p", project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir)));

  expect_the_exported_records();
}

// The darknetxx reads the plain JSON text as well, with a warning.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, reads_the_plain_json_project)
{
  given_the_images();
  given_file(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir)));

  expect_the_exported_records();
}

// The dxxwjz3p project keeps the weights in its network, before the
// annotations: they are decompressed and stepped over.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, reads_the_dxxwjz3p_project_back)
{
  given_the_images();
  given_gzip(dir / "yolov4-obj.dxxwjz3p",
             project_of("dxxwjz3p", dxxwjz3Network));

  Dxxwjz3pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir)));

  expect_the_exported_records();
}

// The darknetxx refuses the documents of another format, and so is the import
// of one: the database is left as it was.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, refuses_the_project_of_another_format)
{
  given_the_images();
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz3p", dxxwjz3Network));

  Dxxwjz2pProject2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context(dir)));
  EXPECT_TRUE(db->get_images_db().empty());

  given_gzip(dir / "other" / "yolov4-obj.dxxwjz3p",
             project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz3pProject2DBImporter dxxwjz3p;

  EXPECT_FALSE(dxxwjz3p.import_db(context(dir / "other")));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_Dxxwjz2pProject2DBImporter, refuses_another_version)
{
  std::string document = project_of("dxxwjz2p", dxxwjz2Network);

  document.replace(document.find("\"version\": 1"), 12, "\"version\": 2");

  given_gzip(dir / "yolov4-obj.dxxwjz2p", document);

  Dxxwjz2pProject2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context(dir)));
}

// A dxxwjz2 weights file is no project: it keeps no annotations.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, refuses_the_document_of_no_annotations)
{
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network, R"(
  "header": {"major": 0, "minor": 2, "revision": 5, "seen": 0})"));

  Dxxwjz2pProject2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context(dir)));
}

TEST_F(UTEST_Dxxwjz2pProject2DBImporter, refuses_the_truncated_file)
{
  given_gzip(dir / "full.dxxwjz2p", project_of("dxxwjz2p", dxxwjz2Network));

  std::ifstream full{dir / "full.dxxwjz2p", std::ios::binary};
  const std::string bytes{std::istreambuf_iterator<char>{full},
                          std::istreambuf_iterator<char>{}};

  given_file(dir / "cut" / "yolov4-obj.dxxwjz2p",
             bytes.substr(0U, bytes.size() / 2U));

  Dxxwjz2pProject2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context(dir / "cut")));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_Dxxwjz2pProject2DBImporter, refuses_a_directory_of_no_project)
{
  Dxxwjz2pProject2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context(dir)));
  EXPECT_FALSE(importer.import_db(context(dir / "missing.dxxwjz2p")));
}

// The darknetxx writes the floats of the pixels; the folders below the one of
// the project are relative to it and the rest absolute; the paths of the
// images may name the sub-folders of their folder.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, reads_the_folders_and_the_float_boxes)
{
  const fs::path absolute = dir / "elsewhere";

  given_file(absolute / "sub/a.png", "not-a-real-image");
  given_gzip(dir / "project/yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network,
                        R"(
  "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
    {"db": {"path": ")" + absolute.string() +
                            R"("}, "annotations": [
      {"rel_path": "sub/a.png", "width": 100, "height": 50, "annotations": [{"name": "dog", "rect": {"x": 10.4, "y": 20.6, "width": 30.5, "height": 1e1}}]}
    ]},
    {"db": {"path": "../images"}, "annotations": [
      {"rel_path": "b.png", "width": 10, "height": 10, "annotations": []}
    ]}
  ]})"));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir / "project")));

  ASSERT_EQ(db->get_images_db().size(), 2U);

  for (const auto& ir : db->get_images_db()) {
    if (ir->path == "sub/a.png") {
      EXPECT_EQ(ir->abs_dir_path, absolute.string());
      ASSERT_EQ(ir->rects.size(), 1U);
      EXPECT_EQ(ir->rects[0]->x, 10);
      EXPECT_EQ(ir->rects[0]->y, 21);
      EXPECT_EQ(ir->rects[0]->width, 31);
      EXPECT_EQ(ir->rects[0]->height, 10);
    } else {
      // the image which is not there is a record of the project all the same
      EXPECT_EQ(ir->path, "b.png");
      EXPECT_EQ(ir->abs_dir_path, (dir / "images").string());
      EXPECT_TRUE(ir->rects.empty());
    }
  }
}

TEST_F(UTEST_Dxxwjz2pProject2DBImporter, skips_the_invalid_boxes_and_images)
{
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network, R"(
  "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
    {"db": {"path": "images"}, "annotations": [
      {"width": 10, "height": 10, "annotations": []},
      {"rel_path": "a.png", "width": 100, "height": 50, "annotations": [
        {"rect": {"x": 1, "y": 1, "width": 1, "height": 1}},
        {"name": "flat", "rect": {"x": 1, "y": 1, "width": 0, "height": 1}},
        {"name": "partial", "rect": {"x": 1, "y": 1}},
        {"name": "kept", "rect": {"x": 1, "y": 1, "width": 2, "height": 2}}
      ]}
    ]},
    {"annotations": []}
  ]})"));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir)));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  ASSERT_EQ(db->get_images_db().front()->rects.size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->rects.front()->name, "kept");
}

// The 0 size of the darknetxx is an unknown one, which the image size reader
// fills in when there is one.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, measures_the_images_of_no_size)
{
  given_file(dir / "images/a.png", "not-a-real-image");
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network, R"(
  "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
    {"db": {"path": "images"}, "annotations": [
      {"rel_path": "a.png", "annotations": []}
    ]}
  ]})"));

  Dxxwjz2pProject2DBImporter importer;

  auto ctx = context(dir);

  ctx->set_image_sizer(std::make_shared<FixedSizer>(64, 32));

  ASSERT_TRUE(importer.import_db(ctx));

  ASSERT_EQ(db->get_images_db().size(), 1U);
  EXPECT_EQ(db->get_images_db().front()->iwidth, 64);
  EXPECT_EQ(db->get_images_db().front()->iheight, 32);
}

// Importing one and the same project twice adds it once.
TEST_F(UTEST_Dxxwjz2pProject2DBImporter, imports_the_same_project_once)
{
  given_the_images();
  given_gzip(dir / "yolov4-obj.dxxwjz2p",
             project_of("dxxwjz2p", dxxwjz2Network));

  Dxxwjz2pProject2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context(dir)));
  ASSERT_TRUE(importer.import_db(context(dir)));

  EXPECT_EQ(db->get_images_db().size(), 2U);
}
