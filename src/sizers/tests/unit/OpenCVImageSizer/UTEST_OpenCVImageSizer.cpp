#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>

#include "src/sizers/ImageSizeFactory.h"
#include "src/sizers/OpenCVImageSizer.h"

using namespace testing;
using iannotator::importers::sizers::create_builtin_image_sizer;
using iannotator::importers::sizers::OpenCVImageSizer;

// This whole suite is configured only in a build that found OpenCV - see the
// tests/unit/CMakeLists.txt beside it. The build without one is covered by
// UTEST_LibFactory, which asserts the factory hands out no reader there.
TEST(UTEST_OpenCVImageSizer, builtin_sizer_is_there_in_an_opencv_build)
{
  EXPECT_NE(create_builtin_image_sizer(), nullptr);
}

namespace
{

namespace fs = std::filesystem;

class UTEST_OpenCVImageSizerF : public Test
{
 public:
  fs::path dir;

  void SetUp() override
  {
    dir = fs::path{testing::TempDir()} / "utest_opencv_sizer";
    fs::remove_all(dir);
    fs::create_directories(dir);
  }

  void TearDown() override { fs::remove_all(dir); }

  /// @brief Writes a solid image of the given size and returns its path.
  std::string given_image(const std::string& name, int width, int height)
  {
    const cv::Mat image{height, width, CV_8UC3, cv::Scalar{10, 20, 30}};

    const std::string fpath = (dir / name).string();

    EXPECT_TRUE(cv::imwrite(fpath, image));

    return fpath;
  }
};

}  // namespace

TEST_F(UTEST_OpenCVImageSizerF, reads_the_size_of_the_image)
{
  const std::string fpath = given_image("src.png", 100, 50);

  int width{0};
  int height{0};

  OpenCVImageSizer sizer;

  ASSERT_TRUE(sizer.read_image_size(fpath, width, height));

  EXPECT_EQ(width, 100);
  EXPECT_EQ(height, 50);
}

TEST_F(UTEST_OpenCVImageSizerF, fails_on_an_image_it_cannot_decode)
{
  std::ofstream{dir / "junk.png"} << "not an image at all";

  int width{0};
  int height{0};

  OpenCVImageSizer sizer;

  EXPECT_FALSE(
      sizer.read_image_size((dir / "junk.png").string(), width, height));

  // the out parameters stay untouched, so a caller sees no half measurement
  EXPECT_EQ(width, 0);
  EXPECT_EQ(height, 0);
}

TEST_F(UTEST_OpenCVImageSizerF, fails_on_a_missing_image_file)
{
  int width{0};
  int height{0};

  OpenCVImageSizer sizer;

  EXPECT_FALSE(
      sizer.read_image_size((dir / "absent.png").string(), width, height));
}

TEST_F(UTEST_OpenCVImageSizerF, rejects_an_empty_path)
{
  int width{0};
  int height{0};

  OpenCVImageSizer sizer;

  EXPECT_FALSE(sizer.read_image_size("", width, height));
}

TEST_F(UTEST_OpenCVImageSizerF, clone_produces_a_usable_sizer)
{
  const std::string fpath = given_image("src.png", 30, 40);

  OpenCVImageSizer sizer;

  auto copy = sizer.clone();

  ASSERT_NE(copy, nullptr);

  int width{0};
  int height{0};

  ASSERT_TRUE(copy->read_image_size(fpath, width, height));

  EXPECT_EQ(width, 30);
  EXPECT_EQ(height, 40);
}
