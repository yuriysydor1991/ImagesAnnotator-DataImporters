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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_FOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_FOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <vector>

#include "src/importers/ImportersAliases.h"

namespace iannotator::importers
{

/**
 * @brief The common half of every importer of the library: everything an
 * import does around the reading of the one layout it implements.
 *
 * The guards over the context, the image measuring instance the context slot
 * or this very library supplies, the merging of the recovered records into the
 * database and the count reported back through the context are the same for
 * all of the layouts, so import_db performs them here and asks the descendant
 * for the records through read_dataset. What tells the importers apart is that
 * one method.
 */
class Folder2DBImporter : virtual public IImporter
{
 public:
  virtual ~Folder2DBImporter() = default;
  Folder2DBImporter() = default;

  virtual bool import_db(IADataImportersContextPtr ictx) override;

 protected:
  /**
   * @brief Reads the dataset of the layout this importer implements.
   *
   * @param importDir The already validated existing source directory.
   * @param records Out: the records recovered out of it. A single unreadable
   * entry is to be logged and left out of this set instead of failing the
   * whole run.
   *
   * @return Should return false only when the directory holds no dataset of
   * this layout at all - a descriptor the layout is built around missing or
   * unreadable - so that a wrong import path is told apart from an empty one.
   * The database is left untouched then.
   */
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) = 0;

  /**
   * @brief Tells whether the layout can be read at all without a measurement.
   *
   * @return Should return true for a layout whose annotations are not stored
   * in the pixels of their image, so that an import with no image size reader
   * fails at once instead of recovering nothing. Returns false here.
   */
  virtual bool needs_image_sizer() const;

  /**
   * @brief Measures the given image with the reader resolved for the running
   * import.
   *
   * @param imagePath The image file to measure.
   * @param width Out: the image width in pixels, left alone on a failure.
   * @param height Out: the image height in pixels, left alone on a failure.
   *
   * @return Returns true when the reader was there and it read both.
   */
  bool measure_image(const std::filesystem::path& imagePath, int& width,
                     int& height);

  /**
   * @brief Tells whether a reader was resolved for the running import at all.
   *
   * @return Returns true when measure_image has anything to ask.
   */
  bool has_image_sizer() const;

  /**
   * @brief Fills the image dimensions of the records which were recovered
   * without them.
   *
   * The layouts whose rectangles are stored in the pixels of their image carry
   * no size of their own, so this is what fills that pair in for them - when
   * there is a reader to ask, since such a record is complete without it. A
   * record which already carries a size keeps the one its dataset declared.
   *
   * @param records The records to look through.
   */
  void measure_unsized(const ImageRecordsSet& records);

  /**
   * @brief Builds the record of the given image file path, split into the
   * directory and the name below it the internal format keeps them in.
   *
   * @param imagePath The image file path the record is to point at.
   *
   * @return Returns the new record, without any rectangle of its own.
   */
  static ImageRecordPtr create_record(const std::filesystem::path& imagePath);

  /**
   * @brief Reads the given file whole.
   *
   * The layouts whose descriptor is one document rather than a list of lines
   * - the JSON and the XML ones - are handed to their reader as that whole
   * text.
   *
   * @param fpath The file to read.
   * @param contents Out: everything the file holds, left alone on a failure.
   *
   * @return Returns true when the file was opened and read through.
   */
  static bool read_file(const std::filesystem::path& fpath,
                        std::string& contents);

  /**
   * @brief Reads the given text file into its lines.
   *
   * @param fpath The file to read.
   *
   * @return Returns the lines, the empty ones and the trailing carriage
   * returns of a file written on MS Windows dropped.
   */
  static std::vector<std::string> read_lines(
      const std::filesystem::path& fpath);

 private:
  /// @brief The reader of the running import: the one of the context, or the
  /// one this build of the library ships. Held for the call only.
  IImageSizeFacilityPtr sizer;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_FOLDER2DBIMPORTER_CLASS_H
