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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3FOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3FOLDER2DBIMPORTER_CLASS_H

#include <string>

#include "src/importers/Darknetxx/Dxxwjz2Folder2DBImporter.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers
{

/**
 * @brief The darknetxx training directory to annotations importer of the
 * dxxwjz3 network: the YOLO v4 directory with the `cfg/yolov4-obj.dxxwjz3`
 * network to train beside its cfg file.
 *
 * The directory is the one the Dxxwjz2Folder2DBImporter reads, while the
 * network is the dxxwjz3 document of the darknetxx
 * (https://github.com/yuriysydor1991/darknetxx), the one of nothing but the
 * JSON data, whose class names are the array of the names of their indexes:
 *
 * @code
 *   "network": {"net": {...}, "layers": [...], "classes": ["cat", "dog"]}
 * @endcode
 */
class Dxxwjz3Folder2DBImporter : public Dxxwjz2Folder2DBImporter
{
 public:
  virtual ~Dxxwjz3Folder2DBImporter() = default;
  Dxxwjz3Folder2DBImporter() = default;

 protected:
  virtual const std::string& format_name() const override;
  virtual int format_version() const override;
  virtual const std::string& network_extension() const override;

  virtual bool read_network_classes(const parsers::JsonValue& classes,
                                    ClassNames& names) const override;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3FOLDER2DBIMPORTER_CLASS_H
