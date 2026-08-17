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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_YOLO4FOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_YOLO4FOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/helpers/TypeHelper.h"
#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"

namespace iannotator::importers
{

/**
 * @brief The yolo4 to annotations importer class.
 *
 * Reads the darknet training directory of the YOLO v4 detector: the class
 * names of `data/obj.names`, the image list of `data/train.txt` and one
 * `<image-stem>.txt` label file per image, whose every line carries a class
 * index and the box centre and size of one rectangle, all four normalised by
 * the size of the image. That normalisation is why the layout can not be read
 * back without measuring the pictures.
 *
 * The `data/obj.data` descriptor is read when it is there and its `names` and
 * `train` entries then decide which files the two lists are taken from, so a
 * dataset laid out by some other tool is followed rather than assumed. The
 * `cfg/yolov4-obj.cfg` network descriptor holds nothing of the annotations and
 * is not read at all.
 */
class Yolo4Folder2DBImporter : public Folder2DBImporter,
                               virtual public helpers::TypeHelper
{
 public:
  virtual ~Yolo4Folder2DBImporter() = default;
  Yolo4Folder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief The boxes are normalised, so the layout is unreadable without a
  /// measurement of every image they belong to.
  virtual bool needs_image_sizer() const override;

 private:
  using ObjData = std::unordered_map<std::string, std::string>;
  using ClassNames = std::vector<std::string>;
  using Paths = std::vector<std::filesystem::path>;

  inline static const std::string dataRel = "data";
  inline static const std::string cfgRel = "cfg";
  inline static const std::string objNamesRel = dataRel + "/obj.names";
  inline static const std::string objDataRel = dataRel + "/obj.data";
  inline static const std::string trainTxtRel = dataRel + "/train.txt";
  inline static const std::string valTxtRel = dataRel + "/val.txt";
  inline static const std::string txtExt = ".txt";

  static ObjData read_obj_data(const std::filesystem::path& importDir);
  static std::filesystem::path resolve(const std::filesystem::path& importDir,
                                       const ObjData& objData,
                                       const std::string& key,
                                       const std::string& fallbackRel);
  static ClassNames read_class_names(const std::filesystem::path& namesPath);
  static Paths read_image_list(const std::filesystem::path& importDir,
                               const std::filesystem::path& listPath);
  static Paths scan_data_directory(const std::filesystem::path& importDir);

  ImageRecordPtr read_image(const std::filesystem::path& imagePath,
                            const ClassNames& classes);
  static void read_label_file(const std::filesystem::path& labelPath,
                              const ClassNames& classes,
                              const ImageRecordPtr& ir);
  static ImageRecordRectPtr read_label_line(const std::string& line,
                                            const ClassNames& classes,
                                            const ImageRecordPtr& ir);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_YOLO4FOLDER2DBIMPORTER_CLASS_H
