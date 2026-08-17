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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PLAINTXTFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PLAINTXTFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"

namespace iannotator::importers
{

/**
 * @brief The plain txt format to annotations importer class.
 *
 * Reads the one `<annotation-name>.txt` file per annotation name the plain
 * text export writes, whose every line names an image, the number of the
 * rectangles of that name drawn over it and their `x y width height` in the
 * image own pixels.
 */
class PlainTxtFolder2DBImporter : public Folder2DBImporter
{
 public:
  virtual ~PlainTxtFolder2DBImporter() = default;
  PlainTxtFolder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& outRecords) override;

 private:
  using Tokens = std::vector<std::string>;
  /// @brief The recovered records by their full image path, so that the very
  /// same image named in the file of every annotation drawn over it collects
  /// all of them into one record
  using Path2Record = std::unordered_map<std::string, ImageRecordPtr>;

  static const std::string txtExt;

  void read_tag_file(const std::filesystem::path& fpath,
                     const std::string& tagName);
  void read_tag_line(const std::string& line, const std::string& tagName);

  ImageRecordPtr get_record(const std::string& imagePath);

  static Tokens tokenize(const std::string& line);
  /**
   * @brief Locates the token holding the rectangles count of the line.
   *
   * The image path is written first and unquoted, so a path holding a space
   * reaches this side as more than one token. What tells the path from the
   * rest is the count itself: it is the first token which is a number with
   * exactly four numbers per rectangle behind it.
   *
   * @param tokens The whole line, split on the whitespace.
   *
   * @return Returns the index of the count token, or the tokens size when the
   * line holds no such token at all.
   */
  static Tokens::size_type find_count_token(const Tokens& tokens);

  Path2Record path2record;
  /// @brief The recovered records in the order the files named them, since an
  /// unordered map keeps none
  ImageRecordsSet records;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PLAINTXTFOLDER2DBIMPORTER_CLASS_H
