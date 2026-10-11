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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ZLIBCONTROLLER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ZLIBCONTROLLER_CLASS_H

#include <memory>
#include <string>
#include <vector>

#include "src/zlib/GzipFileStreams.h"

/**
 * @brief The zlib compression adaptor subsystem namespace.
 *
 * Named after this library instead of the zlibi of the cpp-app-template: the
 * sibling ImagesAnnotator-DataExporters library carries the very same
 * controller, and the std::make_shared instantiations of its classes stay
 * exported whatever the symbol visibility (see ImportersAPI.h), so a namespace
 * of the two libraries would let the dynamic linker bind the calls of one of
 * them to the definitions of the other.
 */
namespace iadi0zlibi
{

/**
 * @brief A small zlib wrapper that hides the streaming boilerplate behind a few
 * simple compress / uncompress methods (in memory and to / from gzip files).
 *
 * The in-memory methods use the zlib (deflate) wrapper; the file methods use
 * the gzip (.gz) container. All methods log and return an empty result / false
 * on a zlib error instead of throwing, so they are trivial to call.
 *
 * The controller of the cpp-app-template
 * (https://github.com/yuriysydor1991/cpp-app-template, the appZlib branch), as
 * the darknetxx (https://github.com/yuriysydor1991/darknetxx) adapted it for
 * its gzip compressed dxxwjz files: the buffers are handed to the zlib in
 * parts, so their size isn't limited by the 32 bits zlib counters, and the
 * gzip files may be written and read a chunk at a time through the standard
 * streams (see the create_file_writer() and the create_file_reader()
 * methods), which the network weights files of the hundreds of megabytes
 * need.
 */
class ZlibController
{
 public:
  using buffer = std::vector<unsigned char>;
  using ZlibControllerPtr = std::shared_ptr<ZlibController>;

  /// @brief The zlib default compression level (the Z_DEFAULT_COMPRESSION).
  static constexpr const int DEFAULT_LEVEL = -1;
  /// @brief The fastest compression level (the Z_BEST_SPEED).
  static constexpr const int BEST_SPEED = 1;
  /// @brief The smallest output compression level (the Z_BEST_COMPRESSION).
  static constexpr const int BEST_COMPRESSION = 9;

  virtual ~ZlibController() = default;
  ZlibController() = default;
  ZlibController(const ZlibController&) = delete;
  ZlibController& operator=(const ZlibController&) = delete;

  /**
   * @brief Compresses a buffer in memory with the zlib (deflate) wrapper.
   *
   * @param input The data to compress (may be empty).
   *
   * @return The compressed bytes, or an empty buffer on a zlib error.
   */
  virtual buffer compress(const buffer& input);

  /**
   * @brief Decompresses a buffer produced by compress().
   *
   * @param input The compressed data.
   *
   * @return The original bytes, or an empty buffer on a zlib error, truncated
   * or empty input.
   */
  virtual buffer uncompress(const buffer& input);

  /**
   * @brief Compresses a buffer and stores it as a gzip (.gz) file.
   *
   * @param path The destination file path.
   * @param input The data to compress.
   *
   * @return Returns true on success and false on any I/O or zlib error.
   */
  virtual bool compress_to_file(const std::string& path, const buffer& input);

  /**
   * @brief Reads and decompresses a gzip (.gz) file.
   *
   * @param path The source file path.
   *
   * @return The decompressed bytes, or an empty buffer on any I/O or zlib
   * error.
   */
  virtual buffer uncompress_from_file(const std::string& path);

  /**
   * @brief Opens the gzip (.gz) file for the write through the output stream,
   * which compresses the data a chunk at a time.
   *
   * @param path The destination file path, truncated.
   * @param level The compression level: the DEFAULT_LEVEL, the BEST_SPEED, the
   * BEST_COMPRESSION or any of the zlib 0 up to 9 ones.
   *
   * @return The open stream, the close() of which tells if the file is
   * complete, or a nullptr for the file unable to open.
   */
  virtual std::unique_ptr<GzipOStream> create_file_writer(
      const std::string& path, int level = DEFAULT_LEVEL);

  /**
   * @brief Opens the gzip (.gz) file for the read through the input stream,
   * which decompresses the data a chunk at a time.
   *
   * @param path The source file path.
   *
   * @return The open stream, the failed() of which tells the corrupted or the
   * truncated compressed data, or a nullptr for the file unable to open.
   */
  virtual std::unique_ptr<GzipIStream> create_file_reader(
      const std::string& path);

  /**
   * @brief Tells the gzip file by the magic bytes it starts with.
   *
   * @param path The file path.
   *
   * @return True for the existing gzip file and false otherwise.
   */
  static bool is_gzip_file(const std::string& path);

  /**
   * @brief Creates a ready to use controller instance.
   *
   * @return The created controller.
   */
  static ZlibControllerPtr create();
};

using ZlibControllerPtr = ZlibController::ZlibControllerPtr;

}  // namespace iadi0zlibi

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ZLIBCONTROLLER_CLASS_H
