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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3PPROJECT2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3PPROJECT2DBIMPORTER_CLASS_H

#include <string>

#include "src/importers/Darknetxx/Dxxwjz2pProject2DBImporter.h"

namespace iannotator::importers
{

/**
 * @brief The darknetxx dxxwjz3p training project to annotations importer.
 *
 * The dxxwjz3p file of the darknetxx
 * (https://github.com/yuriysydor1991/darknetxx) is the dxxwjz2p project of the
 * dxxwjz3 network: the network of the JSON options of its cfg sections instead
 * of the cfg text, the weights of a trained one kept in the layers of the
 * network, while the annotations member is the very dxxwjz2p one this
 * importer reads the way the Dxxwjz2pProject2DBImporter does. The weights come
 * before the annotations in such a file, so they are decompressed and stepped
 * over, a chunk at a time.
 *
 * The import path is the `*.dxxwjz3p` project file or the directory holding
 * it, the `yolov4-obj.dxxwjz3p` of the sibling exporters library first.
 */
class Dxxwjz3pProject2DBImporter : public Dxxwjz2pProject2DBImporter
{
 public:
  virtual ~Dxxwjz3pProject2DBImporter() = default;
  Dxxwjz3pProject2DBImporter() = default;

 protected:
  virtual const std::string& format_name() const override;
  virtual int format_version() const override;
  virtual const std::string& project_extension() const override;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ3PPROJECT2DBIMPORTER_CLASS_H
