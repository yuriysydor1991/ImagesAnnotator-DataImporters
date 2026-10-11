/**
 * The "Simplified BSD License"
 *
 * Copyright (c) 2026, Yurii Sydor (yuriysydor1991@gmail.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/zlib/ZlibController.h"

#include <zlib.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>

#include "src/log/log.h"
#include "src/zlib/GzipFileStreams.h"

namespace iadi0zlibi
{

namespace
{
constexpr std::size_t CHUNK = 16384U;

// zlib's z_stream stores avail_in in a 32-bit uInt and the gzwrite() takes an
// unsigned count and returns an int one, so the buffers are handed to the zlib
// in the parts of this size, which fits all of them, instead of being
// silently truncated (e.g. >= 4 GiB on platforms with a 32-bit uInt).
constexpr std::size_t PART = std::size_t{1} << 30U;

/// @brief The size of the next part of the data from the offset.
std::size_t part_of(const std::size_t size, const std::size_t offset)
{
  return std::min(PART, size - offset);
}

}  // namespace

ZlibController::buffer ZlibController::compress(const buffer& input)
{
  z_stream zs{};
  if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
    LOGE("zlib deflateInit failed");
    return {};
  }

  buffer out;
  std::array<unsigned char, CHUNK> chunk{};
  std::size_t offset = 0U;
  int flush = Z_NO_FLUSH;
  int rc = Z_OK;

  do {
    const std::size_t part = part_of(input.size(), offset);

    zs.next_in = const_cast<Bytef*>(input.data() + offset);
    zs.avail_in = static_cast<uInt>(part);
    offset += part;
    flush = offset == input.size() ? Z_FINISH : Z_NO_FLUSH;

    // the deflate consumes the whole part, the output fills the chunks
    do {
      zs.next_out = chunk.data();
      zs.avail_out = static_cast<uInt>(chunk.size());
      rc = deflate(&zs, flush);
      out.insert(out.end(), chunk.data(),
                 chunk.data() + (chunk.size() - zs.avail_out));
    } while (zs.avail_out == 0U && rc != Z_STREAM_ERROR);
  } while (flush != Z_FINISH && rc != Z_STREAM_ERROR);

  deflateEnd(&zs);

  if (rc != Z_STREAM_END) {
    LOGE("zlib deflate failed with code " << rc);
    return {};
  }

  LOGT("Compressed " << input.size() << " bytes into " << out.size());

  return out;
}

ZlibController::buffer ZlibController::uncompress(const buffer& input)
{
  buffer out;

  if (input.empty()) {
    return out;
  }

  z_stream zs{};
  if (inflateInit(&zs) != Z_OK) {
    LOGE("zlib inflateInit failed");
    return {};
  }

  std::array<unsigned char, CHUNK> chunk{};
  std::size_t offset = 0U;
  int rc = Z_OK;

  do {
    const std::size_t part = part_of(input.size(), offset);

    zs.next_in = const_cast<Bytef*>(input.data() + offset);
    zs.avail_in = static_cast<uInt>(part);
    offset += part;

    do {
      zs.next_out = chunk.data();
      zs.avail_out = static_cast<uInt>(chunk.size());
      rc = inflate(&zs, Z_NO_FLUSH);

      if (rc == Z_NEED_DICT || rc == Z_DATA_ERROR || rc == Z_MEM_ERROR ||
          rc == Z_STREAM_ERROR) {
        inflateEnd(&zs);
        LOGE("zlib inflate failed with code " << rc);
        return {};
      }

      out.insert(out.end(), chunk.data(),
                 chunk.data() + (chunk.size() - zs.avail_out));
    } while (zs.avail_out == 0U && rc != Z_STREAM_END);
  } while (rc != Z_STREAM_END && offset < input.size());

  inflateEnd(&zs);

  if (rc != Z_STREAM_END) {
    LOGE("zlib inflate of the " << input.size()
                                << " bytes ended before the stream end");
    return {};
  }

  LOGT("Decompressed " << input.size() << " bytes into " << out.size());

  return out;
}

bool ZlibController::compress_to_file(const std::string& path,
                                      const buffer& input)
{
  std::unique_ptr<GzipOStream> file = create_file_writer(path);

  if (file == nullptr) {
    return false;
  }

  for (std::size_t offset = 0U; offset < input.size();) {
    const std::size_t part = part_of(input.size(), offset);

    file->write(reinterpret_cast<const char*>(input.data() + offset),
                static_cast<std::streamsize>(part));
    offset += part;
  }

  if (!file->close()) {
    LOGE("Failed to write the gzip file: " << path);
    return false;
  }

  LOGT("Wrote " << input.size() << " bytes into the gzip file " << path);

  return true;
}

ZlibController::buffer ZlibController::uncompress_from_file(
    const std::string& path)
{
  std::unique_ptr<GzipIStream> file = create_file_reader(path);

  if (file == nullptr) {
    return {};
  }

  buffer out;
  std::array<char, CHUNK> chunk{};

  while (file->read(chunk.data(), static_cast<std::streamsize>(chunk.size())) ||
         file->gcount() > 0) {
    out.insert(out.end(), chunk.data(), chunk.data() + file->gcount());
  }

  if (file->failed()) {
    LOGE("Failed to read the gzip file: " << path);
    return {};
  }

  LOGT("Read " << out.size() << " bytes from the gzip file " << path);

  return out;
}

std::unique_ptr<GzipOStream> ZlibController::create_file_writer(
    const std::string& path, const int level)
{
  auto file = std::make_unique<GzipOStream>();

  if (!file->open(path, level)) {
    return {};
  }

  return file;
}

std::unique_ptr<GzipIStream> ZlibController::create_file_reader(
    const std::string& path)
{
  auto file = std::make_unique<GzipIStream>();

  if (!file->open(path)) {
    return {};
  }

  return file;
}

bool ZlibController::is_gzip_file(const std::string& path)
{
  // the ID1 and ID2 bytes of the RFC 1952 gzip member header
  static constexpr const unsigned char GZIP_ID1 = 0x1FU;
  static constexpr const unsigned char GZIP_ID2 = 0x8BU;

  std::ifstream file{path, std::ios::binary};
  std::array<char, 2U> magic{};

  if (!file.read(magic.data(), static_cast<std::streamsize>(magic.size()))) {
    return false;
  }

  return static_cast<unsigned char>(magic[0]) == GZIP_ID1 &&
         static_cast<unsigned char>(magic[1]) == GZIP_ID2;
}

ZlibController::ZlibControllerPtr ZlibController::create()
{
  return std::make_shared<ZlibController>();
}

}  // namespace iadi0zlibi
