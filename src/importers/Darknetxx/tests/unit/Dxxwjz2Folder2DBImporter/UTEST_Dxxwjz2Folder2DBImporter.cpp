#include <ImagesAnnotatorDataDrivers-0.14/IADataDriversFacade.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "src/importers/Darknetxx/Dxxwjz2Folder2DBImporter.h"
#include "src/importers/Darknetxx/Dxxwjz3Folder2DBImporter.h"
#include "src/zlib/ZlibController.h"

using namespace testing;
using iannotator::importers::Dxxwjz2Folder2DBImporter;
using iannotator::importers::Dxxwjz3Folder2DBImporter;
using iannotator::importers::IADataImportersContext;
using iannotator::importers::IAnnotationsDBPtr;
using iannotator::importers::IImageSizeFacility;
using iannotator::importers::IImageSizeFacilityPtr;

namespace iadd = ImagesAnnotatorDataDrivers014;

namespace
{

namespace fs = std::filesystem;

/// @brief Reports one and the same size for every image asked about, the size
/// the normalised boxes are multiplied back by
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

/// @brief The network to train of the classes of the indexes in any order
const std::string dxxwjz2Network = R"({
  "format": "dxxwjz2",
  "version": 2,
  "generator": "darknetxx 0.11.0",
  "network": {"cfg": "[net]\nbatch=64\n", "classes": [
    {"index": 1, "name": "dog"},
    {"index": 0, "name": "cat"}
  ]},
  "training": {"seconds": 0,
    "iteration": [],
    "loss": [],
    "avg_loss": []
  }
})";

const std::string dxxwjz3Network = R"({
  "format": "dxxwjz3",
  "version": 3,
  "generator": "darknetxx 0.11.0",
  "network": {"net": {"batch": 64}, "layers": [
    {"type": "yolo", "options": {"mask": [0,1,2], "classes": 2}}
  ], "classes": ["cat", "dog"]},
  "training": {"seconds": 0,
    "loss": [],
    "avg_loss": []
  }
})";

class UTEST_Dxxwjz2Folder2DBImporter : public Test
{
 public:
  fs::path dir;
  IAnnotationsDBPtr db;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_dxxwjz2_folder_import";
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

    std::ofstream f{fpath, std::ios::binary};
    f << contents;
  }

  void given_gzip(const std::string& relPath, const std::string& document)
  {
    const fs::path fpath = dir / relPath;

    fs::create_directories(fpath.parent_path());

    ASSERT_TRUE(iadi0zlibi::ZlibController::create()->compress_to_file(
        fpath.string(), {document.cbegin(), document.cend()}));
  }

  /// @brief The YOLO v4 directory of one image of the box of the class 1, the
  /// class names file left to the caller
  void given_the_yolo4_directory()
  {
    given_file("data/obj/a.png", "not-a-real-image");
    given_file("data/obj/a.txt", "1 0.5 0.5 0.5 0.25\n");
    given_file("data/train.txt", "data/obj/a.png\n");
  }

  std::shared_ptr<IADataImportersContext> context()
  {
    auto ctx = std::make_shared<IADataImportersContext>();
    ctx->set_import_path(dir.string());
    ctx->set_db(db);
    ctx->set_image_sizer(std::make_shared<FixedSizer>(200, 100));
    return ctx;
  }

  /// @brief The name of the one box of the one image imported
  std::string imported_box()
  {
    if (db->get_images_db().size() != 1U ||
        db->get_images_db().front()->rects.size() != 1U) {
      return {};
    }

    return db->get_images_db().front()->rects.front()->name;
  }
};

}  // namespace

// The darknetxx keeps the class names in the network: the directory of no
// names file is read of them.
TEST_F(UTEST_Dxxwjz2Folder2DBImporter, reads_the_classes_of_the_network)
{
  given_the_yolo4_directory();
  given_gzip("cfg/yolov4-obj.dxxwjz2", dxxwjz2Network);

  Dxxwjz2Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(imported_box(), "dog");

  const auto& rect = db->get_images_db().front()->rects.front();

  EXPECT_EQ(rect->x, 50);
  EXPECT_EQ(rect->y, 38);
  EXPECT_EQ(rect->width, 100);
  EXPECT_EQ(rect->height, 25);
}

TEST_F(UTEST_Dxxwjz2Folder2DBImporter, reads_the_classes_of_the_dxxwjz3_one)
{
  given_the_yolo4_directory();
  given_gzip("cfg/trained.dxxwjz3", dxxwjz3Network);

  Dxxwjz3Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(imported_box(), "dog");
}

// The names file the YOLO v4 export writes beside the network wins, the way
// the darknetxx reads a YOLO4 folder.
TEST_F(UTEST_Dxxwjz2Folder2DBImporter, the_names_file_wins_over_the_network)
{
  given_the_yolo4_directory();
  given_file("data/obj.names", "bird\nfish\n");
  given_gzip("cfg/yolov4-obj.dxxwjz2", dxxwjz2Network);

  Dxxwjz2Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(imported_box(), "fish");
}

// A directory of no network is the YOLO v4 one it extends.
TEST_F(UTEST_Dxxwjz2Folder2DBImporter, reads_the_directory_of_no_network)
{
  given_the_yolo4_directory();
  given_file("data/obj.names", "cat\ndog\n");

  Dxxwjz2Folder2DBImporter importer;

  ASSERT_TRUE(importer.import_db(context()));

  EXPECT_EQ(imported_box(), "dog");
}

TEST_F(UTEST_Dxxwjz2Folder2DBImporter, refuses_the_directory_of_no_classes)
{
  given_the_yolo4_directory();

  Dxxwjz2Folder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

// The network of another format names no classes of this one.
TEST_F(UTEST_Dxxwjz2Folder2DBImporter, reads_no_classes_of_another_format)
{
  given_the_yolo4_directory();
  given_gzip("cfg/yolov4-obj.dxxwjz2", dxxwjz3Network);

  Dxxwjz2Folder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
  EXPECT_TRUE(db->get_images_db().empty());
}

TEST_F(UTEST_Dxxwjz2Folder2DBImporter, refuses_the_classes_of_invalid_indexes)
{
  std::string network = dxxwjz2Network;

  network.replace(network.find("\"index\": 0"), 10, "\"index\": 1");

  given_the_yolo4_directory();
  given_gzip("cfg/yolov4-obj.dxxwjz2", network);

  Dxxwjz2Folder2DBImporter importer;

  EXPECT_FALSE(importer.import_db(context()));
}
