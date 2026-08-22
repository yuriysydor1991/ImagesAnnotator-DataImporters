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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/helpers/TypeHelper.h"
#include "src/importers/ImportersAliases.h"
#include "src/importers/JsonDescriptor2DBImporter.h"

namespace iannotator::importers
{

/**
 * @brief The COCO object detection dataset to annotations importer class.
 *
 * Reads the single JSON descriptor the layout is built around: its `images`
 * array names the pictures together with the size of every one of them, its
 * `categories` array names the annotations, and its `annotations` array holds
 * the `[x, y, width, height]` box of every rectangle in the pixels of the
 * image it was drawn over. Those are the very four numbers an ImageRecordRect
 * carries, so nothing of a rectangle is computed back here.
 *
 * The descriptor carries the image dimensions itself, which is why this is one
 * of the layouts that need no image measuring instance. The `area`, the
 * `iscrowd` and the `segmentation` of an annotation are read over: the first
 * is the box multiplied out, and the annotations database holds neither a
 * crowd flag nor a mask.
 *
 * The directory the sibling exporters library writes - the descriptor under
 * `annotations/instances_default.json` and the pictures under `images/` - is
 * read as it stands. So is a dataset laid out by some other tool: the
 * descriptor is looked for wherever such a tool leaves it, and the pictures in
 * the directory that very descriptor stands beside.
 */
class CocoFolder2DBImporter : public JsonDescriptor2DBImporter,
                              virtual public helpers::TypeHelper
{
 public:
  virtual ~CocoFolder2DBImporter() = default;
  CocoFolder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief The `images` array is what tells a COCO descriptor from any other
  /// JSON file lying beside it
  virtual bool holds_dataset(const JsonValue& root) const override;

 private:
  /// @brief The annotation name of every category identifier the descriptor
  /// declares
  using Categories = std::unordered_map<long long, std::string>;
  /// @brief The record of every image identifier the descriptor declares, so
  /// that an annotation reaches the image it names
  using Records = std::unordered_map<long long, ImageRecordPtr>;

  inline static const std::string annotationsRel = "annotations";
  inline static const std::string imagesRel = "images";
  inline static const std::string defaultDescriptor = "instances_default.json";
  inline static const std::string jsonExt = ".json";
  inline static const std::string imagesKey = "images";
  inline static const std::string categoriesKey = "categories";
  inline static const std::string annotationsKey = "annotations";

  /**
   * @brief Lists the files a COCO descriptor may be found in, most expected
   * first: the name the sibling exporters library writes, then the rest of the
   * `annotations` sub-directory, and finally the import directory itself -
   * which is where the tools handing out a single flat directory leave it.
   *
   * @param importDir The dataset root.
   *
   * @return Returns the candidate paths in the order they are to be tried.
   */
  static Paths descriptor_candidates(const std::filesystem::path& importDir);

  /**
   * @brief Locates the directory the `file_name` entries are relative to.
   *
   * @param importDir The dataset root.
   * @param descriptorPath The descriptor found under it.
   *
   * @return Returns the first of the candidate directories which is there: the
   * `images` one of the exported layout, the one named after the descriptor
   * itself - the `train2017` of the public COCO releases - or the dataset root,
   * which is where a flat directory holds its pictures.
   */
  static std::filesystem::path resolve_images_dir(
      const std::filesystem::path& importDir,
      const std::filesystem::path& descriptorPath);

  void read_categories(const JsonValue& root);
  void read_images(const JsonValue& root,
                   const std::filesystem::path& imagesDir,
                   ImageRecordsSet& records);
  void read_annotations(const JsonValue& root);

  /**
   * @brief Turns one element of the `images` array into a record.
   *
   * @param entry The array element to read.
   * @param imagesDir The directory its `file_name` is relative to.
   *
   * @return Returns the new record, or a nullptr for an entry naming no file.
   */
  ImageRecordPtr read_image(const JsonValue& entry,
                            const std::filesystem::path& imagesDir);

  /**
   * @brief Adds one element of the `annotations` array to the record of the
   * image it names.
   *
   * @param entry The array element to read.
   */
  void read_annotation(const JsonValue& entry);

  Categories categories;
  Records recordsById;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOFOLDER2DBIMPORTER_CLASS_H
