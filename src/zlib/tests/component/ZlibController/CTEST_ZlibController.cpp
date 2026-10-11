#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "src/zlib/GzipFileStreams.h"
#include "src/zlib/ZlibController.h"

using namespace iadi0zlibi;
using buffer = ZlibController::buffer;

namespace
{

buffer filled(std::size_t n, unsigned char value) { return buffer(n, value); }

buffer alternating(std::size_t n)
{
  buffer b(n);
  for (std::size_t i = 0; i < n; ++i) {
    b[i] = static_cast<unsigned char>((i % 2U == 0U) ? 0x00 : 0xFF);
  }
  return b;
}

buffer pseudo_random(std::size_t n)
{
  buffer b(n);
  std::uint32_t state = 0x12345678U;
  for (std::size_t i = 0; i < n; ++i) {
    state = state * 1664525U + 1013904223U;  // numerical recipes LCG
    b[i] = static_cast<unsigned char>(state >> 24);
  }
  return b;
}

buffer from_string(const std::string& s) { return buffer(s.begin(), s.end()); }

buffer read_raw(const std::string& path)
{
  std::ifstream file{path, std::ios::binary};

  return buffer{std::istreambuf_iterator<char>{file},
                std::istreambuf_iterator<char>{}};
}

void write_raw(const std::string& path, const buffer& data)
{
  std::ofstream file{path, std::ios::binary | std::ios::trunc};

  file.write(reinterpret_cast<const char*>(data.data()),
             static_cast<std::streamsize>(data.size()));
}

/// @brief Reads the whole stream, the way a parser consumes it.
buffer read_all(std::istream& stream)
{
  return buffer{std::istreambuf_iterator<char>{stream},
                std::istreambuf_iterator<char>{}};
}

}  // namespace

class CTEST_ZlibController : public testing::Test
{
 protected:
  ZlibController zlib;  // exercises the real zlib library

  ~CTEST_ZlibController() override
  {
    for (const std::string& path : files) {
      std::remove(path.c_str());
    }
  }

  /// @brief The path of the test file, removed at the test end.
  std::string file(const std::string& name)
  {
    const std::string path =
        (std::filesystem::path{CTEST_ZlibController_DATA_DIR} /
         (std::string{
              testing::UnitTest::GetInstance()->current_test_info()->name()} +
          "." + name))
            .string();

    files.push_back(path);

    return path;
  }

  void expect_in_memory_round_trip(const buffer& data)
  {
    const buffer compressed = zlib.compress(data);
    const buffer restored = zlib.uncompress(compressed);
    EXPECT_EQ(restored, data);
  }

  void expect_file_round_trip(const std::string& name, const buffer& data)
  {
    const std::string path = file(name);

    ASSERT_TRUE(zlib.compress_to_file(path, data));
    EXPECT_TRUE(ZlibController::is_gzip_file(path));

    const buffer restored = zlib.uncompress_from_file(path);
    EXPECT_EQ(restored, data);
  }

  /// @brief Writes the data through the stream in the pieces of the size.
  void write_stream(const std::string& path, const buffer& data,
                    std::size_t piece, int level)
  {
    std::unique_ptr<GzipOStream> out = zlib.create_file_writer(path, level);

    ASSERT_NE(out, nullptr);

    for (std::size_t offset = 0U; offset < data.size(); offset += piece) {
      const std::size_t count = std::min(piece, data.size() - offset);

      out->write(reinterpret_cast<const char*>(data.data() + offset),
                 static_cast<std::streamsize>(count));
    }

    EXPECT_TRUE(out->good());
    EXPECT_TRUE(out->close());
    EXPECT_TRUE(out->error().empty()) << out->error();
  }

  std::vector<std::string> files;
};

// --- in-memory compress / uncompress over different data sets ---------------

TEST_F(CTEST_ZlibController, round_trip_empty)
{
  expect_in_memory_round_trip({});
}

TEST_F(CTEST_ZlibController, round_trip_single_byte)
{
  expect_in_memory_round_trip(filled(1, 0x42));
}

TEST_F(CTEST_ZlibController, round_trip_all_zeros)
{
  expect_in_memory_round_trip(filled(100000, 0x00));
}

TEST_F(CTEST_ZlibController, round_trip_all_ones)
{
  expect_in_memory_round_trip(filled(100000, 0xFF));
}

TEST_F(CTEST_ZlibController, round_trip_alternating_zeros_and_ones)
{
  expect_in_memory_round_trip(alternating(100000));
}

TEST_F(CTEST_ZlibController, round_trip_pseudo_random)
{
  expect_in_memory_round_trip(pseudo_random(50000));
}

TEST_F(CTEST_ZlibController, round_trip_short_text)
{
  expect_in_memory_round_trip(from_string("Hello, compressed sentence!"));
}

TEST_F(CTEST_ZlibController, round_trip_medium_text)
{
  std::string text;
  for (int i = 0; i < 100; ++i) {
    text += "The quick brown fox jumps over the lazy dog. ";
  }
  expect_in_memory_round_trip(from_string(text));
}

TEST_F(CTEST_ZlibController, round_trip_large_text)
{
  std::string text;
  for (int i = 0; i < 20000; ++i) {
    text += "line " + std::to_string(i) + ": some repetitive-ish payload\n";
  }
  expect_in_memory_round_trip(from_string(text));
}

TEST_F(CTEST_ZlibController, round_trip_of_many_output_chunks)
{
  // the incompressible data spans the hundreds of the 16 KiB output chunks
  expect_in_memory_round_trip(pseudo_random(5U * 1024U * 1024U + 7U));
}

// --- compression actually shrinks highly compressible data -----------------

TEST_F(CTEST_ZlibController, repetitive_data_is_actually_compressed)
{
  const buffer data = filled(100000, 0x00);
  const buffer compressed = zlib.compress(data);

  EXPECT_FALSE(compressed.empty());
  EXPECT_LT(compressed.size(), data.size());
}

TEST_F(CTEST_ZlibController, uncompress_of_a_truncated_stream_returns_empty)
{
  const buffer compressed = zlib.compress(pseudo_random(100000));

  for (const std::size_t cut :
       {std::size_t{1}, std::size_t{4}, compressed.size() / 2U,
        compressed.size() - 1U}) {
    SCOPED_TRACE("the stream cut to " + std::to_string(cut) + " bytes");

    EXPECT_TRUE(zlib.uncompress(buffer(compressed.begin(),
                                       compressed.begin() +
                                           static_cast<std::ptrdiff_t>(cut)))
                    .empty());
  }
}

// --- gzip file creation round trips ----------------------------------------

TEST_F(CTEST_ZlibController, file_round_trip_text)
{
  expect_file_round_trip(
      "text.gz",
      from_string("Hello, compressed sentence! Stored in a .gz file."));
}

TEST_F(CTEST_ZlibController, file_round_trip_all_zeros)
{
  expect_file_round_trip("zeros.gz", filled(50000, 0x00));
}

TEST_F(CTEST_ZlibController, file_round_trip_pseudo_random)
{
  expect_file_round_trip("random.gz", pseudo_random(50000));
}

TEST_F(CTEST_ZlibController, file_round_trip_empty)
{
  expect_file_round_trip("empty.gz", {});
}

TEST_F(CTEST_ZlibController, file_round_trip_of_many_stream_chunks)
{
  // the data spans the dozens of the 128 KiB chunks of the stream buffers
  expect_file_round_trip("big.gz", pseudo_random(9U * 1024U * 1024U + 13U));
}

TEST_F(CTEST_ZlibController, uncompress_from_missing_file_returns_empty)
{
  EXPECT_TRUE(zlib.uncompress_from_file(file("does_not_exist.gz")).empty());
}

TEST_F(CTEST_ZlibController, compress_to_an_unwritable_path_fails)
{
  EXPECT_FALSE(zlib.compress_to_file(file("no/such/dir/x.gz"), filled(10, 1)));
}

// --- the gzip file streams --------------------------------------------------

TEST_F(CTEST_ZlibController, streams_round_trip_the_pieces_of_every_size)
{
  const buffer data = pseudo_random(3U * GzipStreamBuf::CHUNK + 11U);

  for (const std::size_t piece :
       {std::size_t{1}, std::size_t{7}, std::size_t{4096},
        GzipStreamBuf::CHUNK - 1U, GzipStreamBuf::CHUNK,
        GzipStreamBuf::CHUNK + 1U, data.size()}) {
    SCOPED_TRACE("the pieces of " + std::to_string(piece) + " bytes");

    const std::string path = file("pieces.gz");

    write_stream(path, data, piece, ZlibController::DEFAULT_LEVEL);

    EXPECT_TRUE(ZlibController::is_gzip_file(path));

    std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

    ASSERT_NE(in, nullptr);
    EXPECT_EQ(read_all(*in), data);
    EXPECT_FALSE(in->failed()) << in->error();
    EXPECT_FALSE(in->direct());
    EXPECT_TRUE(in->close());
  }
}

TEST_F(CTEST_ZlibController, streams_write_every_compression_level)
{
  std::string text;
  for (int i = 0; i < 50000; ++i) {
    text += std::to_string(i * 7919 % 10007) + ",";
  }

  const buffer data = from_string(text);

  for (const int level :
       {0, ZlibController::BEST_SPEED, 5, ZlibController::BEST_COMPRESSION,
        ZlibController::DEFAULT_LEVEL}) {
    SCOPED_TRACE("the level " + std::to_string(level));

    const std::string path = file("level.gz");

    write_stream(path, data, 1000U, level);

    EXPECT_EQ(zlib.uncompress_from_file(path), data);
  }

  // the stored data takes more room than the compressed one
  const std::string stored = file("stored.gz");
  const std::string best = file("best.gz");

  write_stream(stored, data, data.size(), 0);
  write_stream(best, data, data.size(), ZlibController::BEST_COMPRESSION);

  EXPECT_GT(std::filesystem::file_size(stored), data.size());
  EXPECT_LT(std::filesystem::file_size(best), data.size() / 2U);
}

TEST_F(CTEST_ZlibController, stream_reads_the_lines_of_the_text)
{
  const std::string path = file("lines.gz");

  write_stream(path, from_string("first line\nsecond line\n\nlast"), 3U,
               ZlibController::DEFAULT_LEVEL);

  std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

  ASSERT_NE(in, nullptr);

  std::vector<std::string> lines;
  std::string line;

  while (std::getline(*in, line)) {
    lines.push_back(line);
  }

  EXPECT_EQ(lines, (std::vector<std::string>{"first line", "second line", "",
                                             "last"}));
  EXPECT_FALSE(in->failed());
}

TEST_F(CTEST_ZlibController, stream_reads_the_plain_file_as_it_is)
{
  const std::string path = file("plain.json");
  const buffer data = from_string("{\"not\": \"compressed\"}");

  write_raw(path, data);

  EXPECT_FALSE(ZlibController::is_gzip_file(path));

  std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

  ASSERT_NE(in, nullptr);
  EXPECT_EQ(read_all(*in), data);
  EXPECT_TRUE(in->direct());
  EXPECT_FALSE(in->failed());
}

TEST_F(CTEST_ZlibController, stream_fails_on_the_truncated_file)
{
  const std::string path = file("truncated.gz");
  const buffer data = pseudo_random(1024U * 1024U);

  write_stream(path, data, data.size(), ZlibController::DEFAULT_LEVEL);

  const buffer whole = read_raw(path);

  // the cuts of the deflate data, of the CRC and of the length trailer
  for (const std::size_t cut :
       {whole.size() / 3U, whole.size() - 8U, whole.size() - 1U}) {
    SCOPED_TRACE("the file cut to " + std::to_string(cut) + " of " +
                 std::to_string(whole.size()) + " bytes");

    write_raw(path, buffer(whole.begin(),
                           whole.begin() + static_cast<std::ptrdiff_t>(cut)));

    std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

    ASSERT_NE(in, nullptr);

    const buffer read = read_all(*in);

    // the cut trailer leaves the whole data readable, but not verified
    EXPECT_LE(read.size(), data.size());
    EXPECT_TRUE(in->failed());
    EXPECT_FALSE(in->error().empty());
    EXPECT_TRUE(zlib.uncompress_from_file(path).empty());
  }
}

TEST_F(CTEST_ZlibController, stream_fails_on_the_corrupted_file)
{
  const std::string path = file("corrupted.gz");
  const buffer data = pseudo_random(256U * 1024U);

  write_stream(path, data, data.size(), ZlibController::DEFAULT_LEVEL);

  const buffer whole = read_raw(path);

  // a byte of the deflate data and a byte of the CRC of the trailer
  for (const std::size_t position : {whole.size() / 2U, whole.size() - 6U}) {
    SCOPED_TRACE("the byte " + std::to_string(position) + " of " +
                 std::to_string(whole.size()) + " flipped");

    buffer corrupted = whole;
    corrupted[position] =
        static_cast<unsigned char>(corrupted[position] ^ 0x5AU);

    write_raw(path, corrupted);

    std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

    ASSERT_NE(in, nullptr);

    read_all(*in);

    EXPECT_TRUE(in->failed());
    EXPECT_TRUE(zlib.uncompress_from_file(path).empty());
  }
}

TEST_F(CTEST_ZlibController, stream_close_reports_the_failed_write)
{
  // every write of the /dev/full fails with no space left on the device
  if (!std::filesystem::exists("/dev/full")) {
    GTEST_SKIP() << "no /dev/full device";
  }

  std::unique_ptr<GzipOStream> out =
      zlib.create_file_writer("/dev/full", ZlibController::DEFAULT_LEVEL);

  ASSERT_NE(out, nullptr);

  const buffer data = pseudo_random(4U * GzipStreamBuf::CHUNK);

  out->write(reinterpret_cast<const char*>(data.data()),
             static_cast<std::streamsize>(data.size()));

  EXPECT_FALSE(out->close());
  EXPECT_FALSE(out->error().empty());
  EXPECT_FALSE(zlib.compress_to_file("/dev/full", data));
}

TEST_F(CTEST_ZlibController, is_gzip_file_tells_the_short_and_the_other_files)
{
  const std::string empty = file("empty");
  const std::string one = file("one");
  const std::string other = file("other");

  write_raw(empty, {});
  write_raw(one, {0x1FU});
  write_raw(other, {0x1FU, 0x8CU, 0x08U});

  EXPECT_FALSE(ZlibController::is_gzip_file(empty));
  EXPECT_FALSE(ZlibController::is_gzip_file(one));
  EXPECT_FALSE(ZlibController::is_gzip_file(other));
  EXPECT_FALSE(ZlibController::is_gzip_file(file("missing")));
}

TEST_F(CTEST_ZlibController, stream_reads_the_concatenated_gzip_members)
{
  const std::string first = file("first.gz");
  const std::string second = file("second.gz");
  const std::string both = file("both.gz");

  write_stream(first, from_string("the first member, "), 5U,
               ZlibController::DEFAULT_LEVEL);
  write_stream(second, pseudo_random(GzipStreamBuf::CHUNK + 3U), 1000U,
               ZlibController::BEST_SPEED);

  buffer joined = read_raw(first);
  const buffer tail = read_raw(second);
  joined.insert(joined.end(), tail.begin(), tail.end());
  write_raw(both, joined);

  buffer expected = from_string("the first member, ");
  const buffer second_data = pseudo_random(GzipStreamBuf::CHUNK + 3U);
  expected.insert(expected.end(), second_data.begin(), second_data.end());

  std::unique_ptr<GzipIStream> in = zlib.create_file_reader(both);

  ASSERT_NE(in, nullptr);
  EXPECT_EQ(read_all(*in), expected);
  EXPECT_FALSE(in->failed()) << in->error();
}

TEST_F(CTEST_ZlibController, stream_fails_on_the_data_after_the_gzip_stream)
{
  const std::string path = file("trailing.gz");

  write_stream(path, from_string("the gzip data"), 100U,
               ZlibController::DEFAULT_LEVEL);

  buffer whole = read_raw(path);
  whole.push_back(0x00U);
  whole.push_back(0x2AU);
  write_raw(path, whole);

  std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

  ASSERT_NE(in, nullptr);

  // the data itself comes first, the failure follows
  EXPECT_EQ(read_all(*in), from_string("the gzip data"));
  EXPECT_TRUE(in->failed());
}

TEST_F(CTEST_ZlibController, stream_fails_on_the_trailer_cut_at_the_chunk_end)
{
  // the zlib gzread() takes the file truncated right after the compressed
  // data for a complete one when the data ends at the boundary of its read,
  // the stream buffer tells them apart
  const std::string path = file("aligned.gz");
  const buffer data = pseudo_random(8U * GzipStreamBuf::CHUNK);

  write_stream(path, data, data.size(), ZlibController::DEFAULT_LEVEL);

  const buffer whole = read_raw(path);

  for (std::size_t cut = whole.size() - 8U; cut < whole.size(); ++cut) {
    SCOPED_TRACE("the trailer cut to " +
                 std::to_string(cut - (whole.size() - 8U)) + " bytes");

    write_raw(path, buffer(whole.begin(),
                           whole.begin() + static_cast<std::ptrdiff_t>(cut)));

    std::unique_ptr<GzipIStream> in = zlib.create_file_reader(path);

    ASSERT_NE(in, nullptr);
    EXPECT_EQ(read_all(*in), data);
    EXPECT_TRUE(in->failed());
  }
}
