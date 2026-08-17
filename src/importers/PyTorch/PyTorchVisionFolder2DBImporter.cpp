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

#include "src/importers/PyTorch/PyTorchVisionFolder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>

#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;
}

bool PyTorchVisionFolder2DBImporter::needs_image_sizer() const { return true; }

bool PyTorchVisionFolder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  for (const auto& entry : sorted_entries(importDir)) {
    if (!fs::is_directory(entry)) {
      LOGT(
          "Skipping the file beside the class directories: " << entry.string());
      continue;
    }

    const std::string tagName = entry.filename().string();

    if (tagName.empty()) {
      LOGE("The directory names no annotation: " << entry.string());
      continue;
    }

    read_tag_directory(entry, tagName, records);
  }

  return true;
}

void PyTorchVisionFolder2DBImporter::read_tag_directory(
    const std::filesystem::path& tagDir, const std::string& tagName,
    ImageRecordsSet& records)
{
  LOGT("Reading the annotation " << tagName << " out of " << tagDir.string());

  for (const auto& entry : sorted_entries(tagDir)) {
    if (!fs::is_regular_file(entry)) {
      LOGT("Skipping the nested directory: " << entry.string());
      continue;
    }

    auto ir = read_crop(entry, tagName);

    if (ir == nullptr) {
      LOGE("Skipping the crop: " << entry.string());
      continue;
    }

    records.emplace_back(ir);
  }
}

ImageRecordPtr PyTorchVisionFolder2DBImporter::read_crop(
    const std::filesystem::path& imagePath, const std::string& tagName)
{
  int width{0};
  int height{0};

  if (!measure_image(imagePath, width, height)) {
    LOGE("The whole area of " << imagePath.string()
                              << " is the annotation, and it stayed unknown");
    return {};
  }

  auto ir = create_record(imagePath);

  assert(ir != nullptr);

  ir->iwidth = width;
  ir->iheight = height;

  // The crop is the annotation: the rectangle covers the file from its very
  // origin, which is where the export cut it out of the picture that is gone.
  ir->rects.emplace_back(
      std::make_shared<ImageRecordRect>(tagName, 0, 0, width, height));

  return ir;
}

PyTorchVisionFolder2DBImporter::Paths
PyTorchVisionFolder2DBImporter::sorted_entries(
    const std::filesystem::path& dirPath)
{
  Paths entries;

  try {
    for (const auto& entry : fs::directory_iterator{dirPath}) {
      entries.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the directory " << dirPath.string()
                                       << " reason: " << e.what());
    return entries;
  }

  // The filesystem hands the entries over in no particular order, so this only
  // keeps one and the same directory importing one and the same way twice.
  std::sort(entries.begin(), entries.end());

  return entries;
}

}  // namespace iannotator::importers
