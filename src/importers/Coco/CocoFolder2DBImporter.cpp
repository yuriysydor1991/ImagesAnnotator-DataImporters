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

#include "src/importers/Coco/CocoFolder2DBImporter.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{

namespace fs = std::filesystem;

/// @brief How many numbers the box of an annotation is written as: x, y,
/// width and height, of the top left corner and in the image own pixels
constexpr const std::size_t boxFields = 4U;

/// @brief Reads an identifier of the descriptor, which is a JSON number while
/// every array of the format keys itself by it
long long to_id(const double& value) { return std::llround(value); }

}  // namespace

bool CocoFolder2DBImporter::read_dataset(const std::filesystem::path& importDir,
                                         ImageRecordsSet& records)
{
  categories.clear();
  recordsById.clear();

  fs::path descriptorPath;

  const JsonValuePtr root =
      read_descriptor(descriptor_candidates(importDir), descriptorPath);

  if (root == nullptr) {
    LOGE("No COCO descriptor found under " << importDir.string());
    return false;
  }

  LOGD("Reading the COCO descriptor " << descriptorPath.string());

  read_categories(*root);

  read_images(*root, resolve_images_dir(importDir, descriptorPath), records);

  read_annotations(*root);

  measure_unsized(records);

  // Nothing of the descriptor is kept beyond the call: the very next import
  // driven with this same importer reads a directory of its own.
  categories.clear();
  recordsById.clear();

  return true;
}

bool CocoFolder2DBImporter::holds_dataset(const JsonValue& root) const
{
  const JsonValue* images = root.is_object() ? root.find(imagesKey) : nullptr;

  return images != nullptr && images->is_array();
}

CocoFolder2DBImporter::Paths CocoFolder2DBImporter::descriptor_candidates(
    const std::filesystem::path& importDir)
{
  Paths candidates;

  const fs::path exported = importDir / annotationsRel / defaultDescriptor;

  if (fs::is_regular_file(exported)) {
    candidates.emplace_back(exported);
  }

  collect_descriptors(importDir / annotationsRel, candidates);
  collect_descriptors(importDir, candidates);

  return candidates;
}

fs::path CocoFolder2DBImporter::resolve_images_dir(
    const std::filesystem::path& importDir,
    const std::filesystem::path& descriptorPath)
{
  const fs::path exported = importDir / imagesRel;

  if (fs::is_directory(exported)) {
    return exported;
  }

  // The public COCO releases name their descriptor after the very directory
  // the pictures of it lie in: instances_train2017.json beside train2017/.
  const std::string stem = descriptorPath.stem().string();

  const auto underscore = stem.find('_');

  if (underscore != std::string::npos && underscore + 1U < stem.size()) {
    const fs::path named = importDir / stem.substr(underscore + 1U);

    if (fs::is_directory(named)) {
      return named;
    }
  }

  return importDir;
}

void CocoFolder2DBImporter::read_categories(const JsonValue& root)
{
  const JsonValue* declared = root.find(categoriesKey);

  if (declared == nullptr || !declared->is_array()) {
    LOGW("The descriptor declares no categories, so its annotations name none");
    return;
  }

  for (const auto& entry : declared->get_items()) {
    assert(entry != nullptr);

    double id{0.0};
    std::string name;

    if (!entry->read_number("id", id) || !entry->read_text("name", name)) {
      LOGE("A category of the descriptor carries no identifier and name pair");
      continue;
    }

    if (name.empty()) {
      LOGE("The category " << to_id(id) << " names no annotation");
      continue;
    }

    categories[to_id(id)] = name;
  }
}

void CocoFolder2DBImporter::read_images(const JsonValue& root,
                                        const std::filesystem::path& imagesDir,
                                        ImageRecordsSet& records)
{
  const JsonValue* declared = root.find(imagesKey);

  assert(declared != nullptr);
  assert(declared->is_array());

  LOGD("Reading the images of the descriptor out of " << imagesDir.string());

  for (const auto& entry : declared->get_items()) {
    assert(entry != nullptr);

    auto ir = read_image(*entry, imagesDir);

    if (ir == nullptr) {
      continue;
    }

    double id{0.0};

    // An image without an identifier reaches no annotation of the descriptor,
    // and is still an image of the project.
    if (entry->read_number("id", id)) {
      recordsById[to_id(id)] = ir;
    } else {
      LOGW("The image " << ir->path << " carries no identifier");
    }

    records.emplace_back(ir);
  }
}

ImageRecordPtr CocoFolder2DBImporter::read_image(
    const JsonValue& entry, const std::filesystem::path& imagesDir)
{
  std::string fileName;

  if (!entry.read_text("file_name", fileName) || fileName.empty()) {
    LOGE("An image of the descriptor names no file");
    return {};
  }

  const fs::path imagePath = imagesDir / fileName;

  if (!fs::is_regular_file(imagePath)) {
    // The record is built all the same: the descriptor holds everything the
    // annotations of that image are made of, and a project whose pictures were
    // moved is repaired by pointing it at them again.
    LOGW("The descriptor names an image which is not there: "
         << imagePath.string());
  }

  auto ir = create_record(imagePath);

  assert(ir != nullptr);

  double width{0.0};
  double height{0.0};

  if (entry.read_number("width", width) &&
      entry.read_number("height", height)) {
    ir->iwidth = toPixels(width);
    ir->iheight = toPixels(height);
  } else {
    LOGW("The image " << fileName << " is declared without a size");
  }

  return ir;
}

void CocoFolder2DBImporter::read_annotations(const JsonValue& root)
{
  const JsonValue* declared = root.find(annotationsKey);

  if (declared == nullptr || !declared->is_array()) {
    LOGW("The descriptor holds no annotations array");
    return;
  }

  for (const auto& entry : declared->get_items()) {
    assert(entry != nullptr);

    read_annotation(*entry);
  }
}

void CocoFolder2DBImporter::read_annotation(const JsonValue& entry)
{
  double imageId{0.0};

  if (!entry.read_number("image_id", imageId)) {
    LOGE("An annotation of the descriptor names no image");
    return;
  }

  auto riter = recordsById.find(to_id(imageId));

  if (riter == recordsById.end()) {
    LOGE("The annotation names the image " << to_id(imageId)
                                           << " the descriptor declares not");
    return;
  }

  double categoryId{0.0};

  if (!entry.read_number("category_id", categoryId)) {
    LOGE("An annotation of the descriptor names no category");
    return;
  }

  auto citer = categories.find(to_id(categoryId));

  if (citer == categories.end()) {
    LOGE("The annotation names the category "
         << to_id(categoryId) << " the descriptor declares not");
    return;
  }

  std::vector<double> box;

  if (!entry.read_numbers("bbox", box) || box.size() != boxFields) {
    LOGE("The annotation of the category " << citer->second
                                           << " carries no box of four "
                                              "numbers");
    return;
  }

  const int width = toPixels(box[2U]);
  const int height = toPixels(box[3U]);

  if (width <= 0 || height <= 0) {
    LOGE("The annotation of the category " << citer->second
                                           << " carries a box of no area");
    return;
  }

  // The box is written the way an ImageRecordRect holds it - the top left
  // corner and the size, in the pixels of the image - so this is the whole
  // reading of it.
  riter->second->rects.emplace_back(std::make_shared<ImageRecordRect>(
      citer->second, toPixels(box[0U]), toPixels(box[1U]), width, height));
}

}  // namespace iannotator::importers
