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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PYTORCHVISIONFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PYTORCHVISIONFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <vector>

#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"

namespace iannotator::importers
{

/**
 * @brief The annotations importer for the PyTorch Vision understandable
 * format. (Which is directories with cropped images, each in it's own
 * category).
 *
 * The layout has lost the pictures the crops were cut out of, so every crop
 * becomes an image record of its own here, carrying the one rectangle over its
 * whole area named after the directory it was found in. That is the reason the
 * import needs a measurement of every file: the extent of that rectangle is
 * the size of the image itself.
 */
class PyTorchVisionFolder2DBImporter : public Folder2DBImporter
{
 public:
  virtual ~PyTorchVisionFolder2DBImporter() = default;
  PyTorchVisionFolder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief The annotation of such a record is the cropped image itself, so
  /// the layout is unreadable without measuring it.
  virtual bool needs_image_sizer() const override;

 private:
  using Paths = std::vector<std::filesystem::path>;

  static Paths sorted_entries(const std::filesystem::path& dirPath);

  void read_tag_directory(const std::filesystem::path& tagDir,
                          const std::string& tagName, ImageRecordsSet& records);
  ImageRecordPtr read_crop(const std::filesystem::path& imagePath,
                           const std::string& tagName);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PYTORCHVISIONFOLDER2DBIMPORTER_CLASS_H
