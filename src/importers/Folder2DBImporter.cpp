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

#include "src/importers/Folder2DBImporter.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "src/log/log.h"
#include "src/sizers/ImageSizeFactory.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;
}

bool Folder2DBImporter::import_db(LibraryContextPtr ictx)
{
  assert(ictx != nullptr);
  assert(!ictx->get_import_path().empty());
  assert(ictx->get_db() != nullptr);

  if (ictx == nullptr) {
    LOGE("Invalid import context pointer provided");
    return false;
  }

  if (ictx->get_import_path().empty()) {
    LOGE("Import context with no import path provided");
    return false;
  }

  if (ictx->get_db() == nullptr) {
    LOGE("Import context with no annotations database");
    return false;
  }

  const fs::path importDir = ictx->get_import_path();

  if (!fs::is_directory(importDir)) {
    LOGE("No directory found under the import path: " << importDir.string());
    return false;
  }

  // A reader the consumer supplied always wins: a project that already decodes
  // images its own way - the GUI toolkits of the ImagesAnnotator application,
  // say - keeps doing exactly that. The library's own reader only fills an
  // empty slot, and is a nullptr itself when this build found no OpenCV.
  sizer = ictx->get_image_sizer();

  if (sizer == nullptr) {
    sizer = sizers::create_builtin_image_sizer();
  }

  if (sizer == nullptr && needs_image_sizer()) {
    LOGE(
        "No image size reader provided and this build of the library ships "
        "none of its own");
    return false;
  }

  LOGD("Will be trying to import some data from " << importDir.string());

  ImageRecordsSet records;

  const bool read = read_dataset(importDir, records);

  // The reader is not kept beyond the call: the context may hand a different
  // one over to the very next import driven with this same importer.
  sizer.reset();

  if (!read) {
    LOGE("No dataset of the wanted layout found under " << importDir.string());
    return false;
  }

  if (records.empty()) {
    LOGW("No annotated image was recovered out of " << importDir.string());
  }

  ictx->get_db()->add_images_db(records);
  ictx->set_imported_records(records.size());

  LOGI("Recovered " << records.size() << " image records out of "
                    << importDir.string());

  return true;
}

bool Folder2DBImporter::needs_image_sizer() const { return false; }

bool Folder2DBImporter::has_image_sizer() const { return sizer != nullptr; }

bool Folder2DBImporter::measure_image(const std::filesystem::path& imagePath,
                                      int& width, int& height)
{
  if (sizer == nullptr) {
    LOGT("No image size reader available for " << imagePath.string());
    return false;
  }

  if (!sizer->read_image_size(imagePath.string(), width, height)) {
    LOGE("Fail to measure the image: " << imagePath.string());
    return false;
  }

  if (width <= 0 || height <= 0) {
    LOGE("The image size reader reported no area for " << imagePath.string());
    return false;
  }

  return true;
}

ImageRecordPtr Folder2DBImporter::create_record(
    const std::filesystem::path& imagePath)
{
  return ImageRecord::create(imagePath.filename().string(),
                             imagePath.parent_path().string());
}

std::vector<std::string> Folder2DBImporter::read_lines(
    const std::filesystem::path& fpath)
{
  std::vector<std::string> lines;

  std::ifstream file(fpath);

  if (!file.is_open()) {
    LOGE("Fail to open the file: " << fpath.string());
    return lines;
  }

  std::string line;

  while (std::getline(file, line)) {
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
      line.pop_back();
    }

    if (line.empty()) {
      continue;
    }

    lines.emplace_back(line);
  }

  return lines;
}

}  // namespace iannotator::importers
