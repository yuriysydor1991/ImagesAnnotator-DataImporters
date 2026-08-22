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

#include "src/importers/Ultralytics/UltralyticsFolder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{

namespace fs = std::filesystem;

/// @brief How many numbers one point of a label file line takes: x and y
constexpr const std::size_t pointFields = 2U;

/**
 * @brief Cuts a normalised coordinate down to the image it belongs to.
 *
 * An Ultralytics release refuses a whole image over a single coordinate
 * outside the range, so this only ever reaches the datasets written by
 * something else - and a box reaching over an edge is cut down there exactly
 * as the exporting side cuts it.
 */
double clamped(const double& value)
{
  return std::min(1.0, std::max(0.0, value));
}

}  // namespace

bool UltralyticsFolder2DBImporter::needs_image_sizer() const { return true; }

bool UltralyticsFolder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  UltralyticsDataYaml descriptor;

  if (!read_descriptor(importDir, descriptor)) {
    LOGE("No Ultralytics YOLO descriptor found under "
         << importDir.string() << " - this is no dataset of the layout");
    return false;
  }

  const Paths imageDirs = resolve_image_dirs(importDir, descriptor);

  if (imageDirs.empty()) {
    LOGE("The descriptor names no images directory which is there");
  }

  for (const auto& dirPath : imageDirs) {
    LOGD("Reading the images of " << dirPath.string());

    for (const auto& imagePath : collect_images(dirPath)) {
      auto ir = read_image(imagePath, descriptor.get_class_names());

      if (ir == nullptr) {
        LOGE("Skipping the image: " << imagePath.string());
        continue;
      }

      records.emplace_back(ir);
    }
  }

  return true;
}

bool UltralyticsFolder2DBImporter::read_descriptor(
    const std::filesystem::path& importDir,
    UltralyticsDataYaml& descriptor) const
{
  for (const auto& candidate : descriptor_candidates(importDir)) {
    if (descriptor.read(read_lines(candidate))) {
      LOGD("Reading the Ultralytics YOLO descriptor " << candidate.string());
      return true;
    }

    LOGD("The file " << candidate.string()
                     << " describes no Ultralytics YOLO dataset");
  }

  return false;
}

UltralyticsFolder2DBImporter::Paths
UltralyticsFolder2DBImporter::descriptor_candidates(
    const std::filesystem::path& importDir)
{
  Paths candidates;

  const fs::path exported = importDir / defaultDescriptor;

  if (fs::is_regular_file(exported)) {
    candidates.emplace_back(exported);
  }

  Paths found;

  try {
    for (const auto& entry : fs::directory_iterator{importDir}) {
      const bool isYaml = entry.path().extension() == yamlExt ||
                          entry.path().extension() == ymlExt;

      if (!entry.is_regular_file() || !isYaml || entry.path() == exported) {
        continue;
      }

      found.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the import directory " << importDir.string()
                                              << " reason: " << e.what());
    return candidates;
  }

  // The filesystem hands the entries over in no particular order, so this only
  // keeps one and the same directory importing one and the same way twice.
  std::sort(found.begin(), found.end());

  candidates.insert(candidates.end(), found.cbegin(), found.cend());

  return candidates;
}

UltralyticsFolder2DBImporter::Paths
UltralyticsFolder2DBImporter::resolve_image_dirs(
    const std::filesystem::path& importDir,
    const UltralyticsDataYaml& descriptor)
{
  Paths dirs;

  const std::string& root = descriptor.get_root();

  for (const auto& named : {descriptor.get_train(), descriptor.get_val()}) {
    if (named.empty()) {
      continue;
    }

    const fs::path relative{named};

    if (relative.is_absolute()) {
      add_image_dir(relative, dirs);
      continue;
    }

    // The import path wins over the path entry, which names where a directory
    // that was moved no longer is.
    if (!add_image_dir(importDir / relative, dirs) && !root.empty()) {
      add_image_dir(fs::path{root} / relative, dirs);
    }
  }

  // The descriptor of a dataset laid out by hand may name neither part, while
  // the directories of the layout are where they always are.
  add_image_dir(importDir / trainRel, dirs);
  add_image_dir(importDir / imagesRel, dirs);

  return dirs;
}

bool UltralyticsFolder2DBImporter::add_image_dir(
    const std::filesystem::path& dirPath, Paths& dirs)
{
  if (std::find(dirs.cbegin(), dirs.cend(), dirPath) != dirs.cend()) {
    return true;
  }

  if (!fs::is_directory(dirPath)) {
    return false;
  }

  dirs.emplace_back(dirPath);

  return true;
}

UltralyticsFolder2DBImporter::Paths
UltralyticsFolder2DBImporter::collect_images(
    const std::filesystem::path& dirPath)
{
  Paths images;

  try {
    for (const auto& entry : fs::directory_iterator{dirPath}) {
      // The layout names no image extension anywhere, so everything which is
      // not a label file is a picture of it.
      if (!entry.is_regular_file() || entry.path().extension() == txtExt) {
        continue;
      }

      images.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the images directory " << dirPath.string()
                                              << " reason: " << e.what());
    return images;
  }

  std::sort(images.begin(), images.end());

  return images;
}

fs::path UltralyticsFolder2DBImporter::label_path_of(
    const std::filesystem::path& imagePath)
{
  const std::string labelName = imagePath.stem().string() + txtExt;

  Paths elements;
  Paths::size_type swapAt{0U};
  bool found{false};

  for (const auto& element : imagePath.parent_path()) {
    if (element == imagesRel) {
      swapAt = elements.size();
      found = true;
    }

    elements.emplace_back(element);
  }

  if (!found) {
    // A dataset which keeps its labels beside its images, the way the darknet
    // layout does, is read the way it lies.
    return imagePath.parent_path() / labelName;
  }

  fs::path labelPath;

  for (Paths::size_type iter = 0U; iter < elements.size(); ++iter) {
    labelPath /= iter == swapAt ? fs::path{labelsRel} : elements[iter];
  }

  return labelPath / labelName;
}

ImageRecordPtr UltralyticsFolder2DBImporter::read_image(
    const std::filesystem::path& imagePath, const ClassNames& classes)
{
  int width{0};
  int height{0};

  if (!measure_image(imagePath, width, height)) {
    LOGE("The boxes of " << imagePath.string()
                         << " are normalised by a size which stayed unknown");
    return {};
  }

  auto ir = create_record(imagePath);

  assert(ir != nullptr);

  ir->iwidth = width;
  ir->iheight = height;

  const fs::path labelPath = label_path_of(imagePath);

  if (!fs::is_regular_file(labelPath)) {
    // A training run reads an image without a label file as a picture holding
    // none of the classes, and such an image belongs to the project just as
    // much.
    LOGW("No label file found for " << imagePath.string());
    return ir;
  }

  read_label_file(labelPath, classes, ir);

  return ir;
}

void UltralyticsFolder2DBImporter::read_label_file(
    const std::filesystem::path& labelPath, const ClassNames& classes,
    const ImageRecordPtr& ir) const
{
  assert(ir != nullptr);

  for (const auto& line : read_lines(labelPath)) {
    auto irr = read_label_line(line, classes, ir);

    if (irr == nullptr) {
      LOGE("Fail to read the label line: " << line << " of "
                                           << labelPath.string());
      continue;
    }

    ir->rects.emplace_back(irr);
  }
}

ImageRecordRectPtr UltralyticsFolder2DBImporter::read_label_line(
    const std::string& line, const ClassNames& classes,
    const ImageRecordPtr& ir) const
{
  assert(ir != nullptr);
  assert(ir->iwidth > 0);
  assert(ir->iheight > 0);

  std::istringstream iss{line};

  long long classIndex{0};

  if (!(iss >> classIndex)) {
    LOGT("The line carries no class index");
    return {};
  }

  auto citer = classes.find(classIndex);

  if (citer == classes.end()) {
    LOGE("The class index " << classIndex << " names none of the "
                            << classes.size() << " declared classes");
    return {};
  }

  std::vector<double> values;

  double value{0.0};

  while (iss >> value) {
    values.emplace_back(value);
  }

  NormalizedRect nrect;

  if (!read_rectangle(values, nrect)) {
    return {};
  }

  // The very division the export performed, undone: every coordinate is
  // multiplied back by the image it was normalised by.
  const double left = clamped(nrect.left) * toD(ir->iwidth);
  const double top = clamped(nrect.top) * toD(ir->iheight);
  const double right = clamped(nrect.right) * toD(ir->iwidth);
  const double bottom = clamped(nrect.bottom) * toD(ir->iheight);

  const int width = toPixels(right - left);
  const int height = toPixels(bottom - top);

  if (width <= 0 || height <= 0) {
    LOGE("The " << citer->second << " box covers no area of the image");
    return {};
  }

  return std::make_shared<ImageRecordRect>(citer->second, toPixels(left),
                                           toPixels(top), width, height);
}

bool UltralyticsFolder2DBImporter::read_corners(
    const std::vector<double>& values, NormalizedRect& nrect)
{
  if (values.empty() || values.size() % pointFields != 0U) {
    return false;
  }

  NormalizedRect read;

  read.left = values[0U];
  read.right = values[0U];
  read.top = values[1U];
  read.bottom = values[1U];

  for (std::size_t iter = pointFields; iter < values.size();
       iter += pointFields) {
    read.left = std::min(read.left, values[iter]);
    read.right = std::max(read.right, values[iter]);
    read.top = std::min(read.top, values[iter + 1U]);
    read.bottom = std::max(read.bottom, values[iter + 1U]);
  }

  nrect = read;

  return true;
}

}  // namespace iannotator::importers
