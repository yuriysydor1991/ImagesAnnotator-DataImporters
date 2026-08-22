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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_CREATEMLFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_CREATEMLFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>

#include "src/helpers/TypeHelper.h"
#include "src/importers/ImportersAliases.h"
#include "src/importers/JsonDescriptor2DBImporter.h"

namespace iannotator::importers
{

/**
 * @brief The Create ML object detection dataset to annotations importer class.
 *
 * Reads the flat directory of the Apple `MLObjectDetector`
 * `directoryWithImagesAndJsonAnnotation` data source: the pictures and the one
 * JSON descriptor beside them, whose every element names an image together
 * with the boxes drawn over it.
 *
 * The `x` and the `y` of a box are its centre and not its corner, counted in
 * the image own pixels, which is the one halving this import undoes. They are
 * a JSON number each, so the centre of a box of an odd size lands on a half
 * pixel - as the exporting side leaves it - and the origin this rounds it onto
 * is the very pixel that box was drawn from.
 *
 * Both spellings of the two keys are read: the `imagefilename` and
 * `annotation` pair Apple documents, and the plural `image` and `annotations`
 * one some converters emit instead.
 *
 * The layout carries no image size anywhere, so the dimensions of a record are
 * filled in from the image measuring instance when the consumer supplied one -
 * and the record is complete without it, since the boxes are held in the very
 * pixels the internal format keeps them in.
 */
class CreateMLFolder2DBImporter : public JsonDescriptor2DBImporter,
                                  virtual public helpers::TypeHelper
{
 public:
  virtual ~CreateMLFolder2DBImporter() = default;
  CreateMLFolder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief An array whose elements name an image file is what tells a Create
  /// ML descriptor from any other JSON file lying beside it
  virtual bool holds_dataset(const JsonValue& root) const override;

 private:
  inline static const std::string defaultDescriptor = "annotations.json";
  inline static const std::string fileNameKey = "imagefilename";
  inline static const std::string annotationKey = "annotation";
  /// @brief The plural spelling of the two keys above, which is the dialect of
  /// the format some converters emit
  inline static const std::string pluralFileNameKey = "image";
  inline static const std::string pluralAnnotationKey = "annotations";
  inline static const std::string labelKey = "label";
  inline static const std::string coordinatesKey = "coordinates";

  /**
   * @brief Lists the files a Create ML descriptor may be found in, most
   * expected first: the name the data source demands, then the rest of the
   * directory - which is where an export whose project held an image of that
   * very name left it.
   *
   * @param importDir The dataset root.
   *
   * @return Returns the candidate paths in the order they are to be tried.
   */
  static Paths descriptor_candidates(const std::filesystem::path& importDir);

  /**
   * @brief Reads the image file name of one descriptor element.
   *
   * @param entry The element to read.
   * @param fileName Out: the read name, left alone when there is none.
   *
   * @return Returns true when the element carried either spelling of the key.
   */
  static bool read_file_name(const JsonValue& entry, std::string& fileName);

  /**
   * @brief Looks the annotations array of one descriptor element up.
   *
   * @param entry The element to look into.
   *
   * @return Returns the array under either spelling of the key, or a nullptr
   * when the element carries none.
   */
  static const JsonValue* find_annotations(const JsonValue& entry);

  /**
   * @brief Turns one element of the descriptor into a record.
   *
   * @param entry The element to read.
   * @param importDir The directory its image file name is relative to.
   *
   * @return Returns the new record, or a nullptr for an element naming no
   * file.
   */
  ImageRecordPtr read_image(const JsonValue& entry,
                            const std::filesystem::path& importDir);

  /**
   * @brief Adds one annotation of an element to the record of its image.
   *
   * @param entry The annotation to read.
   * @param ir In-out: the record the rectangle is added to.
   */
  static void read_annotation(const JsonValue& entry, const ImageRecordPtr& ir);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_CREATEMLFOLDER2DBIMPORTER_CLASS_H
