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

#include "src/importers/PascalVoc/PascalVocFolder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>

#include "src/helpers/StringHelper.h"
#include "src/log/log.h"
#include "src/parsers/xml/XmlParser.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;
}

bool PascalVocFolder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  const Paths descriptors = collect_descriptors(importDir);

  if (descriptors.empty()) {
    LOGE("No Pascal VOC descriptor found under " << importDir.string());
    return false;
  }

  auto parser = parsers::XmlParser::create();

  assert(parser != nullptr);

  for (const auto& descriptorPath : descriptors) {
    auto ir = read_descriptor(descriptorPath, importDir, *parser);

    if (ir == nullptr) {
      LOGE("Skipping the descriptor: " << descriptorPath.string());
      continue;
    }

    records.emplace_back(ir);
  }

  // Every descriptor of this layout carries the size of its image, so this
  // only ever reaches the ones a tool wrote without it.
  measure_unsized(records);

  return true;
}

PascalVocFolder2DBImporter::Paths
PascalVocFolder2DBImporter::collect_descriptors(
    const std::filesystem::path& importDir)
{
  Paths descriptors;

  const fs::path devkit = importDir / annotationsRel;

  // A flat directory of the descriptors beside their pictures is the shape
  // LabelImg saves its own work in, so the sub-directory is only where they
  // are looked for first.
  const fs::path dirPath = fs::is_directory(devkit) ? devkit : importDir;

  try {
    for (const auto& entry : fs::directory_iterator{dirPath}) {
      if (!entry.is_regular_file() || entry.path().extension() != xmlExt) {
        continue;
      }

      descriptors.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the directory " << dirPath.string()
                                       << " reason: " << e.what());
    return descriptors;
  }

  // The filesystem hands the entries over in no particular order, so this only
  // keeps one and the same directory importing one and the same way twice.
  std::sort(descriptors.begin(), descriptors.end());

  return descriptors;
}

ImageRecordPtr PascalVocFolder2DBImporter::read_descriptor(
    const std::filesystem::path& descriptorPath,
    const std::filesystem::path& importDir, parsers::IXmlParser& parser)
{
  std::string document;

  if (!read_file(descriptorPath, document)) {
    return {};
  }

  const parsers::XmlNodePtr annotation = parser.parse(document);

  if (annotation == nullptr || annotation->get_name() != rootElement) {
    LOGE("The file " << descriptorPath.string()
                     << " holds no Pascal VOC annotation element");
    return {};
  }

  std::string fileName;

  if (!annotation->read_text(fileNameElement, fileName)) {
    LOGE("The descriptor " << descriptorPath.string() << " names no image");
    return {};
  }

  fileName = helpers::StringHelper::trim(fileName);

  if (fileName.empty()) {
    LOGE("The descriptor " << descriptorPath.string() << " names no image");
    return {};
  }

  auto ir = create_record(
      resolve_image_path(*annotation, descriptorPath, importDir, fileName));

  assert(ir != nullptr);

  read_size(*annotation, ir);

  read_objects(*annotation, ir);

  return ir;
}

fs::path PascalVocFolder2DBImporter::resolve_image_path(
    const XmlNode& annotation, const std::filesystem::path& descriptorPath,
    const std::filesystem::path& importDir, const std::string& fileName)
{
  Paths candidates;

  candidates.emplace_back(importDir / imagesRel);

  std::string folder;

  if (annotation.read_text(folderElement, folder)) {
    folder = helpers::StringHelper::trim(folder);

    if (!folder.empty()) {
      candidates.emplace_back(importDir / folder);
    }
  }

  candidates.emplace_back(descriptorPath.parent_path());
  candidates.emplace_back(importDir);

  for (const auto& candidate : candidates) {
    if (fs::is_regular_file(candidate / fileName)) {
      return candidate / fileName;
    }
  }

  // The record is built all the same: the descriptor holds every rectangle of
  // that image, and a project whose pictures were moved is repaired by
  // pointing it at them again.
  LOGW("The descriptor " << descriptorPath.string()
                         << " names an image which is not there: " << fileName);

  return candidates.front() / fileName;
}

void PascalVocFolder2DBImporter::read_size(const XmlNode& annotation,
                                           const ImageRecordPtr& ir)
{
  assert(ir != nullptr);

  const XmlNode* size = annotation.find(sizeElement);

  if (size == nullptr) {
    LOGW("The image " << ir->path << " is described without a size");
    return;
  }

  double width{0.0};
  double height{0.0};

  if (!size->read_number("width", width) ||
      !size->read_number("height", height)) {
    LOGE("The size of the image " << ir->path << " holds no width and height");
    return;
  }

  ir->iwidth = toPixels(width);
  ir->iheight = toPixels(height);
}

void PascalVocFolder2DBImporter::read_objects(const XmlNode& annotation,
                                              const ImageRecordPtr& ir)
{
  assert(ir != nullptr);

  for (const auto& child : annotation.get_children()) {
    assert(child != nullptr);

    if (child->get_name() != objectElement) {
      continue;
    }

    auto irr = read_object(*child);

    if (irr == nullptr) {
      LOGE("Skipping an object of the image " << ir->path);
      continue;
    }

    ir->rects.emplace_back(irr);
  }
}

ImageRecordRectPtr PascalVocFolder2DBImporter::read_object(
    const XmlNode& object)
{
  std::string name;

  if (!object.read_text(nameElement, name)) {
    LOGE("An object element names no annotation");
    return {};
  }

  name = helpers::StringHelper::trim(name);

  if (name.empty()) {
    LOGE("An object element names no annotation");
    return {};
  }

  const XmlNode* box = object.find(boxElement);

  if (box == nullptr) {
    LOGE("The object " << name << " carries no bndbox element");
    return {};
  }

  double xmin{0.0};
  double ymin{0.0};
  double xmax{0.0};
  double ymax{0.0};

  if (!box->read_number("xmin", xmin) || !box->read_number("ymin", ymin) ||
      !box->read_number("xmax", xmax) || !box->read_number("ymax", ymax)) {
    LOGE("The bndbox of the object " << name << " holds no four corners");
    return {};
  }

  // The two corner points the rectangle was drawn between, turned back into
  // the origin and the size an ImageRecordRect holds.
  const int width = toPixels(xmax - xmin);
  const int height = toPixels(ymax - ymin);

  if (width <= 0 || height <= 0) {
    LOGE("The bndbox of the object " << name << " covers no area");
    return {};
  }

  return std::make_shared<ImageRecordRect>(name, toPixels(xmin), toPixels(ymin),
                                           width, height);
}

}  // namespace iannotator::importers
