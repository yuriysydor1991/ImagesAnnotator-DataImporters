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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2FOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2FOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>

#include "src/importers/Yolo4/Yolo4Folder2DBImporter.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers
{

/**
 * @brief The darknetxx training directory to annotations importer: the YOLO
 * v4 (darknet) directory with the dxxwjz2 network to train beside its cfg
 * file.
 *
 * The darknetxx (https://github.com/yuriysydor1991/darknetxx) trains the very
 * YOLO v4 directory and keeps the class names of the network in its dxxwjz2
 * files, the gzip compressed JSON documents of the cfg text and of the class
 * objects of the `index` and the `name`:
 *
 * @code
 *   "network": {"cfg": "[net]\nbatch=64\n...", "classes": [
 *     {"index": 0, "name": "cat"},
 *     {"index": 1, "name": "dog"}
 *   ]}
 * @endcode
 *
 * The directory is read the way the Yolo4Folder2DBImporter reads it - the
 * images list, the label files, the boxes normalised by the size of their
 * images - while the class names the label files number by are the ones of
 * the `data/obj.names` file, or the ones of the `cfg/yolov4-obj.dxxwjz2`
 * network (the first `.dxxwjz2` one of the `cfg` directory by the name else)
 * for the directory of no names file: the very order the darknetxx takes them
 * in when it reads a YOLO4 folder. The two of the other names are warned of.
 * A directory of no network is read as the YOLO v4 one it is.
 *
 * The descendants read the directories of the later darknetxx networks (see
 * the Dxxwjz3Folder2DBImporter).
 */
class Dxxwjz2Folder2DBImporter : public Yolo4Folder2DBImporter
{
 public:
  virtual ~Dxxwjz2Folder2DBImporter() = default;
  Dxxwjz2Folder2DBImporter() = default;

 protected:
  virtual ClassNames read_classes(
      const std::filesystem::path& importDir,
      const std::filesystem::path& namesPath) override;

  /// @brief The value of the format key of the network.
  virtual const std::string& format_name() const;
  /// @brief The value of the version key of the network.
  virtual int format_version() const;
  /// @brief The file name extension the darknetxx tells the network by.
  virtual const std::string& network_extension() const;

  /**
   * @brief Reads the class names out of the classes member of the network.
   *
   * @param classes The `classes` value of the `network` member.
   * @param names Out: the names in the order of their indexes.
   *
   * @return Returns false for the classes of no valid names.
   */
  virtual bool read_network_classes(const parsers::JsonValue& classes,
                                    ClassNames& names) const;

 private:
  /// @brief The network file of the directory, an empty path for none.
  std::filesystem::path locate_network(
      const std::filesystem::path& importDir) const;
  bool read_network(const std::filesystem::path& networkPath,
                    ClassNames& names) const;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2FOLDER2DBIMPORTER_CLASS_H
