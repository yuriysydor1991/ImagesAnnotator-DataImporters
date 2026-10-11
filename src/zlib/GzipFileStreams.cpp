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

#include "src/zlib/GzipFileStreams.h"

#include <zlib.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "src/log/log.h"

namespace iadi0zlibi
{

namespace
{

/// @brief The deflate and inflate window bits of the gzip wrapper.
constexpr const int GZIP_WINDOW_BITS = 15 + 16;

/// @brief The deflate memory level of the zlib defaults.
constexpr const int DEFLATE_MEMORY_LEVEL = 8;

/// @brief The ID1 and ID2 bytes of the RFC 1952 gzip member header.
constexpr const unsigned char GZIP_ID1 = 0x1FU;
constexpr const unsigned char GZIP_ID2 = 0x8BU;

std::string errno_reason(const int error_number)
{
  return error_number != 0 ? std::string{std::strerror(error_number)}
                           : std::string{};
}

std::string zlib_reason(const z_stream& zs, const int code)
{
  if (zs.msg != nullptr) {
    return zs.msg;
  }

  return "zlib error " + std::to_string(code);
}

}  // namespace

struct GzipStreamBuf::Codec
{
  std::FILE* file{nullptr};
  z_stream zs{};
  /// the zlib stream is initialized: the deflate of the write or the inflate
  /// of the read
  bool zs_ready{false};
  bool deflating{false};
  /// the read of the file, which isn't a gzip one
  bool direct{false};
  /// the read reached the end of the file
  bool input_eof{false};
  /// the read completed the gzip member
  bool member_end{false};
  /// the compressed data chunk
  std::vector<unsigned char> io;

  ~Codec()
  {
    end_stream();

    if (file != nullptr) {
      std::fclose(file);
    }
  }

  void end_stream()
  {
    if (!zs_ready) {
      return;
    }

    zs_ready = false;

    if (deflating) {
      deflateEnd(&zs);
    } else {
      inflateEnd(&zs);
    }
  }
};

GzipStreamBuf::GzipStreamBuf() = default;

GzipStreamBuf::~GzipStreamBuf()
{
  // the owner which cares about the result closes the file on its own
  close();
}

bool GzipStreamBuf::open(const std::string& path, const Mode mode,
                         const int level)
{
  if (mcodec != nullptr) {
    LOGE("The gzip file " << mpath << " is already open");
    return false;
  }

  mpath = path;
  mmode = mode;
  mfailed = false;
  merror.clear();

  auto codec = std::make_unique<Codec>();

  errno = 0;
  codec->file = std::fopen(path.c_str(), mode == Mode::Write ? "wb" : "rb");

  if (codec->file == nullptr) {
    fail(mode == Mode::Write ? "Failed to open the gzip file for writing"
                             : "Failed to open the gzip file for reading",
         errno_reason(errno));
    return false;
  }

  codec->io.assign(CHUNK, 0U);
  mbuffer.assign(CHUNK, '\0');

  if (mode == Mode::Write) {
    const int ret = deflateInit2(
        &codec->zs,
        level < 0 ? Z_DEFAULT_COMPRESSION : std::min(level, Z_BEST_COMPRESSION),
        Z_DEFLATED, GZIP_WINDOW_BITS, DEFLATE_MEMORY_LEVEL, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK) {
      fail("Failed to initialize the gzip compression",
           zlib_reason(codec->zs, ret));
      return false;
    }

    codec->zs_ready = true;
    codec->deflating = true;
    mcodec = std::move(codec);

    setp(mbuffer.data(), mbuffer.data() + mbuffer.size());
    setg(nullptr, nullptr, nullptr);

    return true;
  }

  // the first chunk of the file tells the gzip one by its magic bytes
  const std::size_t count =
      std::fread(codec->io.data(), 1U, codec->io.size(), codec->file);

  if (count < codec->io.size()) {
    if (std::ferror(codec->file) != 0) {
      fail("Failed to read the gzip file", errno_reason(errno));
      return false;
    }

    codec->input_eof = true;
  }

  codec->zs.next_in = codec->io.data();
  codec->zs.avail_in = static_cast<uInt>(count);
  codec->direct =
      count < 2U || codec->io[0] != GZIP_ID1 || codec->io[1] != GZIP_ID2;

  if (!codec->direct) {
    const int ret = inflateInit2(&codec->zs, GZIP_WINDOW_BITS);

    if (ret != Z_OK) {
      fail("Failed to initialize the gzip decompression",
           zlib_reason(codec->zs, ret));
      return false;
    }

    codec->zs_ready = true;
  }

  mcodec = std::move(codec);

  // the empty get area, the first read fills it
  setg(mbuffer.data(), mbuffer.data(), mbuffer.data());
  setp(nullptr, nullptr);

  return true;
}

bool GzipStreamBuf::close()
{
  if (mcodec == nullptr) {
    return !mfailed;
  }

  Codec& codec = *mcodec;

  if (mmode == Mode::Write) {
    // the rest of the data, the end of the deflate stream and the trailer
    if (write_buffer()) {
      codec.zs.next_in = nullptr;
      codec.zs.avail_in = 0U;

      int ret = Z_OK;

      do {
        codec.zs.next_out = codec.io.data();
        codec.zs.avail_out = static_cast<uInt>(codec.io.size());

        ret = deflate(&codec.zs, Z_FINISH);

        const std::size_t have = codec.io.size() - codec.zs.avail_out;

        errno = 0;

        if (have > 0U &&
            std::fwrite(codec.io.data(), 1U, have, codec.file) != have) {
          fail("Failed to write the gzip file", errno_reason(errno));
          break;
        }
      } while (ret == Z_OK);

      if (!mfailed && ret != Z_STREAM_END) {
        fail("Failed to finalize the gzip compression",
             zlib_reason(codec.zs, ret));
      }
    }
  }

  codec.end_stream();

  errno = 0;

  // the close writes the data the C library keeps, the failed write of it
  // (no space left on the device, for example) fails the file
  const int closed = std::fclose(codec.file);

  codec.file = nullptr;

  if (mmode == Mode::Write && closed != 0) {
    fail("Failed to finalize the gzip file", errno_reason(errno));
  }

  mcodec.reset();

  setp(nullptr, nullptr);
  setg(nullptr, nullptr, nullptr);
  mbuffer.clear();
  mbuffer.shrink_to_fit();

  return !mfailed;
}

bool GzipStreamBuf::is_open() const { return mcodec != nullptr; }

bool GzipStreamBuf::failed() const { return mfailed; }

const std::string& GzipStreamBuf::error() const { return merror; }

bool GzipStreamBuf::direct() const
{
  return mcodec != nullptr && mmode == Mode::Read && mcodec->direct;
}

GzipStreamBuf::int_type GzipStreamBuf::overflow(const int_type ch)
{
  if (mcodec == nullptr || mmode != Mode::Write || mfailed) {
    return traits_type::eof();
  }

  if (!write_buffer()) {
    return traits_type::eof();
  }

  if (!traits_type::eq_int_type(ch, traits_type::eof())) {
    *pptr() = traits_type::to_char_type(ch);
    pbump(1);
  }

  return traits_type::not_eof(ch);
}

int GzipStreamBuf::sync()
{
  if (mcodec == nullptr || mfailed) {
    return -1;
  }

  // the data is handed over to the compression, no flush point: they make
  // the compression worse
  if (mmode == Mode::Write && !write_buffer()) {
    return -1;
  }

  return 0;
}

GzipStreamBuf::int_type GzipStreamBuf::underflow()
{
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }

  if (mcodec == nullptr || mmode != Mode::Read || mfailed) {
    return traits_type::eof();
  }

  const std::size_t count = read_buffer();

  if (count == 0U) {
    return traits_type::eof();
  }

  setg(mbuffer.data(), mbuffer.data(),
       mbuffer.data() + static_cast<std::ptrdiff_t>(count));

  return traits_type::to_int_type(*gptr());
}

bool GzipStreamBuf::write_buffer()
{
  if (mfailed) {
    return false;
  }

  Codec& codec = *mcodec;

  codec.zs.next_in = reinterpret_cast<Bytef*>(pbase());
  // the put area is the CHUNK at most, which fits the zlib counts
  codec.zs.avail_in = static_cast<uInt>(pptr() - pbase());

  // the deflate consumes the whole put area, the output fills the chunks
  while (codec.zs.avail_in > 0U) {
    codec.zs.next_out = codec.io.data();
    codec.zs.avail_out = static_cast<uInt>(codec.io.size());

    const int ret = deflate(&codec.zs, Z_NO_FLUSH);

    if (ret != Z_OK && ret != Z_BUF_ERROR) {
      fail("Failed to compress the gzip file", zlib_reason(codec.zs, ret));
      return false;
    }

    const std::size_t have = codec.io.size() - codec.zs.avail_out;

    errno = 0;

    if (have > 0U &&
        std::fwrite(codec.io.data(), 1U, have, codec.file) != have) {
      fail("Failed to write the gzip file", errno_reason(errno));
      return false;
    }
  }

  setp(mbuffer.data(), mbuffer.data() + mbuffer.size());

  return true;
}

std::size_t GzipStreamBuf::read_buffer()
{
  Codec& codec = *mcodec;

  if (codec.direct) {
    // the bytes of the first chunk, then the rest of the file as it is
    if (codec.zs.avail_in > 0U) {
      const std::size_t count =
          std::min<std::size_t>(codec.zs.avail_in, mbuffer.size());

      std::memcpy(mbuffer.data(), codec.zs.next_in, count);
      codec.zs.next_in += count;
      codec.zs.avail_in -= static_cast<uInt>(count);

      return count;
    }

    if (codec.input_eof) {
      return 0U;
    }

    errno = 0;

    const std::size_t count =
        std::fread(mbuffer.data(), 1U, mbuffer.size(), codec.file);

    if (count < mbuffer.size()) {
      if (std::ferror(codec.file) != 0) {
        fail("Failed to read the file", errno_reason(errno));
        return 0U;
      }

      codec.input_eof = true;
    }

    return count;
  }

  while (true) {
    if (codec.zs.avail_in == 0U && !codec.input_eof) {
      errno = 0;

      const std::size_t count =
          std::fread(codec.io.data(), 1U, codec.io.size(), codec.file);

      if (count < codec.io.size()) {
        if (std::ferror(codec.file) != 0) {
          fail("Failed to read the gzip file", errno_reason(errno));
          return 0U;
        }

        codec.input_eof = true;
      }

      codec.zs.next_in = codec.io.data();
      codec.zs.avail_in = static_cast<uInt>(count);
    }

    if (codec.member_end) {
      if (codec.zs.avail_in == 0U) {
        if (codec.input_eof) {
          // the whole gzip stream is read and verified
          return 0U;
        }

        continue;
      }

      // the data after the gzip member is the next gzip member, the inflate
      // fails on the header of anything else
      inflateReset(&codec.zs);
      codec.member_end = false;
    }

    codec.zs.next_out = reinterpret_cast<Bytef*>(mbuffer.data());
    codec.zs.avail_out = static_cast<uInt>(mbuffer.size());

    const int ret = inflate(&codec.zs, Z_NO_FLUSH);

    if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR ||
        ret == Z_STREAM_ERROR) {
      fail("Failed to decompress the gzip file", zlib_reason(codec.zs, ret));
      return 0U;
    }

    // the member is complete once its trailer (the CRC and the size of the
    // data) is verified
    if (ret == Z_STREAM_END) {
      codec.member_end = true;
    }

    const std::size_t produced = mbuffer.size() - codec.zs.avail_out;

    if (produced > 0U) {
      return produced;
    }

    if (!codec.member_end && codec.zs.avail_in == 0U && codec.input_eof) {
      fail("Failed to read the gzip file", "unexpected end of file");
      return 0U;
    }
  }
}

void GzipStreamBuf::fail(const std::string& what, const std::string& reason)
{
  if (mfailed) {
    return;
  }

  mfailed = true;
  merror = what + ": " + mpath + (reason.empty() ? "" : " (" + reason + ")");

  LOGE(merror);
}

GzipOStream::GzipOStream() : std::ostream{nullptr} { rdbuf(&mbuffer); }

bool GzipOStream::open(const std::string& path, const int level)
{
  if (!mbuffer.open(path, GzipStreamBuf::Mode::Write, level)) {
    setstate(std::ios::failbit);
    return false;
  }

  clear();

  return true;
}

bool GzipOStream::close()
{
  const bool closed = mbuffer.close();

  if (!closed) {
    setstate(std::ios::badbit);
  }

  return closed;
}

bool GzipOStream::is_open() const { return mbuffer.is_open(); }

const std::string& GzipOStream::error() const { return mbuffer.error(); }

GzipIStream::GzipIStream() : std::istream{nullptr} { rdbuf(&mbuffer); }

bool GzipIStream::open(const std::string& path)
{
  if (!mbuffer.open(path, GzipStreamBuf::Mode::Read, -1)) {
    setstate(std::ios::failbit);
    return false;
  }

  clear();

  return true;
}

bool GzipIStream::close() { return mbuffer.close(); }

bool GzipIStream::is_open() const { return mbuffer.is_open(); }

bool GzipIStream::failed() const { return mbuffer.failed(); }

const std::string& GzipIStream::error() const { return mbuffer.error(); }

bool GzipIStream::direct() const { return mbuffer.direct(); }

}  // namespace iadi0zlibi
