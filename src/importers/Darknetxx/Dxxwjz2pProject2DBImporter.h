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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PPROJECT2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PPROJECT2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>

#include "src/helpers/TypeHelper.h"
#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers
{

/**
 * @brief The darknetxx dxxwjz2p training project to annotations importer.
 *
 * The dxxwjz2p file of the darknetxx
 * (https://github.com/yuriysydor1991/darknetxx) is the gzip compressed JSON
 * document of the whole YOLO v4 training project but the images: the network,
 * the class names, the training record, the weights of a trained one and the
 * `annotations` member, the ImagesAnnotator project structure of the 0.12.0
 * version the darknetxx reads and writes:
 *
 * @code
 *   "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
 *     {"db": {"path": "images"}, "annotations": [
 *       {"rel_path": "park.jpg", "width": 640, "height": 480, "annotations":
 *         [{"name": "dog", "rect": {"x": 200, "y": 130, "width": 48,
 *         "height": 52}}]}
 *     ]}
 *   ]}
 * @endcode
 *
 * That member alone is read (see the DxxwjzFileReader): the folder of the
 * images of the path relative to the folder of the project file (or
 * absolute), every image of the path relative to its folder, its size and its
 * boxes, of the top left corner and the size in the pixels of the image - the
 * very four numbers of an ImageRecordRect, rounded onto the pixels, since the
 * darknetxx writes them as the floats. The `validation` flag of an image and
 * the `scale_factor` keys mean nothing to the annotations database and are
 * not read.
 *
 * The import path is the project file, or the directory holding it: the
 * `yolov4-obj.dxxwjz2p` the sibling exporters library writes, or else the
 * first `*.dxxwjz2p` file by the name. The project keeps the size of every
 * image of the boxes, the image size reader fills in the size of the rest when
 * there is one.
 *
 * The descendants read the projects of the later darknetxx formats (see the
 * Dxxwjz3pProject2DBImporter).
 */
class Dxxwjz2pProject2DBImporter : public Folder2DBImporter,
                                   virtual public helpers::TypeHelper
{
 public:
  virtual ~Dxxwjz2pProject2DBImporter() = default;
  Dxxwjz2pProject2DBImporter() = default;

 protected:
  /// @brief The project file or the directory holding it.
  virtual bool accepts_import_path(
      const std::filesystem::path& importPath) const override;

  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief The value of the format key of the project.
  virtual const std::string& format_name() const;
  /// @brief The value of the version key of the project.
  virtual int format_version() const;
  /// @brief The file name extension the darknetxx tells the project by.
  virtual const std::string& project_extension() const;

 private:
  using JsonValue = parsers::JsonValue;

  /// @brief The name of the project file the sibling exporters library writes,
  /// but its extension
  inline static const std::string projectStem = "yolov4-obj";
  inline static const std::string annotationsKey = "annotations";

  /**
   * @brief The project file of the import path: the path itself, or the
   * project file of the directory it names.
   *
   * @return Returns an empty path for the directory of no project file.
   */
  std::filesystem::path locate_project(
      const std::filesystem::path& importPath) const;

  void read_folder(const JsonValue& folder,
                   const std::filesystem::path& projectDir,
                   ImageRecordsSet& records) const;
  static ImageRecordPtr read_image(const JsonValue& entry,
                                   const std::filesystem::path& folderPath);
  static ImageRecordRectPtr read_box(const JsonValue& entry);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PPROJECT2DBIMPORTER_CLASS_H
