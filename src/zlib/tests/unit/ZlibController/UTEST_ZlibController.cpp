#include <gtest/gtest.h>

#include "src/zlib/ZlibController.h"

using namespace iadi0zlibi;

TEST(UTEST_ZlibController, create_returns_an_instance)
{
  EXPECT_NE(ZlibController::create(), nullptr);
}

TEST(UTEST_ZlibController, empty_input_round_trips_to_empty)
{
  ZlibController zlib;

  const ZlibController::buffer compressed = zlib.compress({});
  const ZlibController::buffer restored = zlib.uncompress(compressed);

  EXPECT_FALSE(compressed.empty());
  EXPECT_TRUE(restored.empty());
}

TEST(UTEST_ZlibController, uncompress_of_garbage_returns_empty)
{
  ZlibController zlib;

  // Not a valid zlib stream - the controller must fail gracefully (empty).
  const ZlibController::buffer garbage{0x01, 0x02, 0x03, 0x04, 0x05};

  EXPECT_TRUE(zlib.uncompress(garbage).empty());
}

TEST(UTEST_ZlibController, uncompress_of_a_truncated_stream_returns_empty)
{
  ZlibController zlib;

  ZlibController::buffer compressed =
      zlib.compress(ZlibController::buffer(1000U, 0x42));

  ASSERT_GT(compressed.size(), 4U);

  compressed.resize(compressed.size() - 4U);

  EXPECT_TRUE(zlib.uncompress(compressed).empty());
}

TEST(UTEST_ZlibController, file_streams_of_a_missing_directory_are_nullptrs)
{
  ZlibController zlib;

  EXPECT_EQ(zlib.create_file_writer("/no/such/directory/of/the/test.gz"),
            nullptr);
  EXPECT_EQ(zlib.create_file_reader("/no/such/directory/of/the/test.gz"),
            nullptr);
  EXPECT_FALSE(ZlibController::is_gzip_file("/no/such/directory/test.gz"));
}
