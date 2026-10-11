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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZFILEREADER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZFILEREADER_CLASS_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "src/parsers/json/JsonMembersPicker.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers
{

/**
 * @brief Reads the members of a darknetxx dxxwjz file an import needs.
 *
 * The darknetxx (https://github.com/yuriysydor1991/darknetxx) keeps its
 * networks and its training projects in the dxxwjz files, the gzip compressed
 * JSON documents of the `format` and the `version` keys of their own. The file
 * of a trained network keeps every weight of it, hundreds of megabytes of the
 * JSON text, so the file is decompressed a chunk at a time by the gzip file
 * stream of the ZlibController (the one of the cpp-app-template) and the
 * JsonMembersPicker steps over every member but the wanted ones, of which the
 * JsonParser reads the values alone. The reading stops once the wanted members
 * are read: the darknetxx writes the annotations of a dxxwjz2p project before
 * the weights, so the weights of such a project are not even decompressed.
 *
 * The darknetxx reads a file of the plain JSON text as well, with a warning,
 * and so does this reader. The file of another format or version is refused,
 * as the darknetxx refuses it.
 */
class DxxwjzFileReader
{
 public:
  using Path = parsers::JsonMembersPicker::Path;
  using Paths = std::vector<Path>;
  /// @brief The values read, by the paths of their members
  using Members = std::map<Path, parsers::JsonValuePtr>;

  /// @brief The keys every dxxwjz document carries
  inline static const std::string formatKey{"format"};
  inline static const std::string versionKey{"version"};

  /**
   * @param nformat The value of the format key the file has to carry.
   * @param nversion The value of the version key the file has to carry.
   */
  DxxwjzFileReader(std::string nformat, int nversion);
  virtual ~DxxwjzFileReader() = default;

  /**
   * @brief Reads the wanted members of the file.
   *
   * @param fpath The dxxwjz file.
   * @param wanted The paths of the wanted members.
   * @param members Out: the values of the wanted members the file keeps, the
   * ones it keeps none of left out.
   *
   * @return Returns false for the file unable to be read, a corrupted one, the
   * one of another format or version and the one of no JSON document.
   */
  bool read(const std::filesystem::path& fpath, const Paths& wanted,
            Members& members) const;

 private:
  static std::string join(const Path& path);

  std::string format;
  int version;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZFILEREADER_CLASS_H
