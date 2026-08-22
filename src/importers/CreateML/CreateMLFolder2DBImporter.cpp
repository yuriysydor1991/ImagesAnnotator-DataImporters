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

#include "src/importers/CreateML/CreateMLFolder2DBImporter.h"

#include <cassert>
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

bool CreateMLFolder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  fs::path descriptorPath;

  const JsonValuePtr root =
      read_descriptor(descriptor_candidates(importDir), descriptorPath);

  if (root == nullptr) {
    LOGE("No Create ML descriptor found under " << importDir.string());
    return false;
  }

  LOGD("Reading the Create ML descriptor " << descriptorPath.string());

  for (const auto& entry : root->get_items()) {
    assert(entry != nullptr);

    auto ir = read_image(*entry, importDir);

    if (ir == nullptr) {
      continue;
    }

    records.emplace_back(ir);
  }

  measure_unsized(records);

  return true;
}

bool CreateMLFolder2DBImporter::holds_dataset(const JsonValue& root) const
{
  if (!root.is_array()) {
    return false;
  }

  // A dataset of no image at all is a dataset all the same, and the only one
  // whose descriptor names nothing this could be told by.
  if (root.get_items().empty()) {
    return true;
  }

  const JsonValue* first = root.get_items().front().get();

  std::string fileName;

  return first != nullptr && first->is_object() &&
         read_file_name(*first, fileName);
}

CreateMLFolder2DBImporter::Paths
CreateMLFolder2DBImporter::descriptor_candidates(
    const std::filesystem::path& importDir)
{
  Paths candidates;

  const fs::path exported = importDir / defaultDescriptor;

  if (fs::is_regular_file(exported)) {
    candidates.emplace_back(exported);
  }

  collect_descriptors(importDir, candidates);

  return candidates;
}

bool CreateMLFolder2DBImporter::read_file_name(const JsonValue& entry,
                                               std::string& fileName)
{
  return entry.read_text(fileNameKey, fileName) ||
         entry.read_text(pluralFileNameKey, fileName);
}

const CreateMLFolder2DBImporter::JsonValue*
CreateMLFolder2DBImporter::find_annotations(const JsonValue& entry)
{
  const JsonValue* annotations = entry.find(annotationKey);

  if (annotations == nullptr) {
    annotations = entry.find(pluralAnnotationKey);
  }

  return annotations != nullptr && annotations->is_array() ? annotations
                                                           : nullptr;
}

ImageRecordPtr CreateMLFolder2DBImporter::read_image(
    const JsonValue& entry, const std::filesystem::path& importDir)
{
  std::string fileName;

  if (!read_file_name(entry, fileName) || fileName.empty()) {
    LOGE("An element of the descriptor names no image file");
    return {};
  }

  const fs::path imagePath = importDir / fileName;

  if (!fs::is_regular_file(imagePath)) {
    // The record is built all the same: the descriptor holds everything the
    // annotations of that image are made of, and a project whose pictures were
    // moved is repaired by pointing it at them again.
    LOGW("The descriptor names an image which is not there: "
         << imagePath.string());
  }

  auto ir = create_record(imagePath);

  assert(ir != nullptr);

  const JsonValue* annotations = find_annotations(entry);

  if (annotations == nullptr) {
    // An image whose every rectangle was dropped by the export reaches the
    // descriptor with an empty array, and belongs to the project just as much.
    LOGW("The image " << fileName << " carries no annotation array");
    return ir;
  }

  for (const auto& annotation : annotations->get_items()) {
    assert(annotation != nullptr);

    read_annotation(*annotation, ir);
  }

  return ir;
}

void CreateMLFolder2DBImporter::read_annotation(const JsonValue& entry,
                                                const ImageRecordPtr& ir)
{
  assert(ir != nullptr);

  std::string label;

  if (!entry.read_text(labelKey, label) || label.empty()) {
    LOGE("An annotation of the image " << ir->path << " names no label");
    return;
  }

  const JsonValue* coordinates = entry.find(coordinatesKey);

  if (coordinates == nullptr || !coordinates->is_object()) {
    LOGE("The annotation " << label << " of the image " << ir->path
                           << " carries no coordinates");
    return;
  }

  double centreX{0.0};
  double centreY{0.0};
  double width{0.0};
  double height{0.0};

  if (!coordinates->read_number("x", centreX) ||
      !coordinates->read_number("y", centreY) ||
      !coordinates->read_number("width", width) ||
      !coordinates->read_number("height", height)) {
    LOGE("The annotation " << label << " of the image " << ir->path
                           << " carries no whole box");
    return;
  }

  const int boxWidth = toPixels(width);
  const int boxHeight = toPixels(height);

  if (boxWidth <= 0 || boxHeight <= 0) {
    LOGE("The annotation " << label << " of the image " << ir->path
                           << " carries a box of no area");
    return;
  }

  // The very halving the exporting side performed, undone: the origin an
  // ImageRecordRect is drawn from is the centre less half of the size.
  ir->rects.emplace_back(std::make_shared<ImageRecordRect>(
      label, toPixels(centreX - (width / 2.0)),
      toPixels(centreY - (height / 2.0)), boxWidth, boxHeight));
}

}  // namespace iannotator::importers
