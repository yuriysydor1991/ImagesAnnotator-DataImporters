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

#include "src/importers/Yolo4/Yolo4Folder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "src/helpers/StringHelper.h"
#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;
}

bool Yolo4Folder2DBImporter::needs_image_sizer() const { return true; }

bool Yolo4Folder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  const ObjData objData = read_obj_data(importDir);

  const fs::path namesPath = resolve(importDir, objData, "names", objNamesRel);

  const ClassNames classes = read_class_names(namesPath);

  if (classes.empty()) {
    LOGE("No class names read out of " << namesPath.string()
                                       << " - this is no YOLO v4 dataset");
    return false;
  }

  Paths images = read_image_list(
      importDir, resolve(importDir, objData, "train", trainTxtRel));

  if (images.empty()) {
    LOGW("The training list names no image, falling back to the val one");

    images = read_image_list(importDir,
                             resolve(importDir, objData, "valid", valTxtRel));
  }

  if (images.empty()) {
    LOGW("Neither list names an image, falling back to the data directory");

    images = scan_data_directory(importDir);
  }

  for (const auto& imagePath : images) {
    auto ir = read_image(imagePath, classes);

    if (ir == nullptr) {
      LOGE("Skipping the image: " << imagePath.string());
      continue;
    }

    records.emplace_back(ir);
  }

  return true;
}

Yolo4Folder2DBImporter::ObjData Yolo4Folder2DBImporter::read_obj_data(
    const std::filesystem::path& importDir)
{
  ObjData objData;

  // The export writes the descriptor into the data sub-directory, while the
  // darknet examples keep it beside it, so both places are looked at.
  fs::path dataPath = importDir / objDataRel;

  if (!fs::is_regular_file(dataPath)) {
    dataPath = importDir / "obj.data";
  }

  if (!fs::is_regular_file(dataPath)) {
    LOGD("No obj.data descriptor found under " << importDir.string()
                                               << ", assuming the defaults");
    return objData;
  }

  for (const auto& line : read_lines(dataPath)) {
    const auto eq = line.find('=');

    if (eq == std::string::npos) {
      LOGT("Skipping the obj.data line without an equals sign: " << line);
      continue;
    }

    const std::string key = helpers::StringHelper::trim(line.substr(0U, eq));
    const std::string value = helpers::StringHelper::trim(line.substr(eq + 1U));

    if (key.empty() || value.empty()) {
      continue;
    }

    objData[key] = value;
  }

  return objData;
}

fs::path Yolo4Folder2DBImporter::resolve(const std::filesystem::path& importDir,
                                         const ObjData& objData,
                                         const std::string& key,
                                         const std::string& fallbackRel)
{
  auto miter = objData.find(key);

  const fs::path named =
      miter != objData.end() ? fs::path{miter->second} : fs::path{fallbackRel};

  return named.is_absolute() ? named : importDir / named;
}

Yolo4Folder2DBImporter::ClassNames Yolo4Folder2DBImporter::read_class_names(
    const std::filesystem::path& namesPath)
{
  // The line number is the class index every label file writes, so the order
  // of the file is the whole meaning of it.
  return read_lines(namesPath);
}

Yolo4Folder2DBImporter::Paths Yolo4Folder2DBImporter::read_image_list(
    const std::filesystem::path& importDir,
    const std::filesystem::path& listPath)
{
  Paths images;

  if (!fs::is_regular_file(listPath)) {
    LOGD("No image list found under " << listPath.string());
    return images;
  }

  for (const auto& line : read_lines(listPath)) {
    // The list is written relative to the dataset root, which is what makes a
    // darknet directory movable, but an absolute path is accepted as well.
    const fs::path named{line};

    images.emplace_back(named.is_absolute() ? named : importDir / named);
  }

  return images;
}

Yolo4Folder2DBImporter::Paths Yolo4Folder2DBImporter::scan_data_directory(
    const std::filesystem::path& importDir)
{
  Paths images;

  const fs::path dataPath = importDir / dataRel;

  if (!fs::is_directory(dataPath)) {
    LOGE("No data directory found under " << importDir.string());
    return images;
  }

  try {
    for (const auto& entry : fs::directory_iterator{dataPath}) {
      // Everything that is not one of the descriptors the layout is made of
      // is an image of it: the layout names no image extension anywhere, so
      // there is nothing else to go by.
      if (!entry.is_regular_file() || entry.path().extension() == txtExt ||
          entry.path().extension() == ".names" ||
          entry.path().extension() == ".data") {
        continue;
      }

      images.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the data directory " << dataPath.string()
                                            << " reason: " << e.what());
    return images;
  }

  std::sort(images.begin(), images.end());

  return images;
}

ImageRecordPtr Yolo4Folder2DBImporter::read_image(
    const std::filesystem::path& imagePath, const ClassNames& classes)
{
  if (!fs::is_regular_file(imagePath)) {
    LOGE("No file found under the path: " << imagePath.string());
    return {};
  }

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

  const fs::path labelPath =
      imagePath.parent_path() / (imagePath.stem().string() + txtExt);

  if (!fs::is_regular_file(labelPath)) {
    // Darknet reads an image without a label file as a picture holding none of
    // the classes, and such an image belongs to the project just as much.
    LOGW("No label file found for " << imagePath.string());
    return ir;
  }

  read_label_file(labelPath, classes, ir);

  return ir;
}

void Yolo4Folder2DBImporter::read_label_file(
    const std::filesystem::path& labelPath, const ClassNames& classes,
    const ImageRecordPtr& ir)
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

ImageRecordRectPtr Yolo4Folder2DBImporter::read_label_line(
    const std::string& line, const ClassNames& classes,
    const ImageRecordPtr& ir)
{
  assert(ir != nullptr);
  assert(ir->iwidth > 0);
  assert(ir->iheight > 0);

  std::istringstream iss{line};

  std::size_t classIndex{0U};
  double ncx{0.0};
  double ncy{0.0};
  double nwidth{0.0};
  double nheight{0.0};

  if (!(iss >> classIndex >> ncx >> ncy >> nwidth >> nheight)) {
    LOGT("The line holds no class index and four numbers behind it");
    return {};
  }

  if (classIndex >= classes.size()) {
    LOGE("The class index " << classIndex << " names none of the "
                            << classes.size() << " known classes");
    return {};
  }

  // The very division the export performed, undone: the centre and the size
  // are multiplied back by the image they were normalised by, and the origin
  // is the centre less half of the size again.
  const double cx = ncx * toD(ir->iwidth);
  const double cy = ncy * toD(ir->iheight);
  const double width = nwidth * toD(ir->iwidth);
  const double height = nheight * toD(ir->iheight);

  return std::make_shared<ImageRecordRect>(
      classes[classIndex], toPixels(cx - (width / 2.0)),
      toPixels(cy - (height / 2.0)), toPixels(width), toPixels(height));
}

}  // namespace iannotator::importers
