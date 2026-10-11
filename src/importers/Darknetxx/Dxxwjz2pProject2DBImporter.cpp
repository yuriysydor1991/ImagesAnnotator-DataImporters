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

#include "src/importers/Darknetxx/Dxxwjz2pProject2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "src/importers/Darknetxx/DxxwjzFileReader.h"
#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;

const std::string dxxwjz2pFormat{"dxxwjz2p"};
const std::string dxxwjz2pExtension{".dxxwjz2p"};
constexpr const int dxxwjz2pVersion{1};
}  // namespace

bool Dxxwjz2pProject2DBImporter::accepts_import_path(
    const std::filesystem::path& importPath) const
{
  if (!fs::is_regular_file(importPath) && !fs::is_directory(importPath)) {
    LOGE("No " << project_extension()
               << " project file nor directory found under the import path: "
               << importPath.string());
    return false;
  }

  return true;
}

bool Dxxwjz2pProject2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& records)
{
  const fs::path project = locate_project(importDir);

  if (project.empty()) {
    LOGE("No " << project_extension() << " project file found under "
               << importDir.string());
    return false;
  }

  LOGD("Reading the " << format_name() << " project " << project.string());

  DxxwjzFileReader::Members members;

  if (!DxxwjzFileReader{format_name(), format_version()}.read(
          project, {{annotationsKey}}, members)) {
    return false;
  }

  const auto found = members.find({annotationsKey});

  if (found == members.end() || found->second == nullptr) {
    LOGE("The file " << project.string() << " keeps no annotations: this is no "
                     << format_name() << " project");
    return false;
  }

  const JsonValue& annotations = *found->second;
  const JsonValue* folders = annotations.find(annotationsKey);

  if (folders == nullptr || !folders->is_array()) {
    LOGE("The annotations of the project " << project.string()
                                           << " keep no folders of the images");
    return false;
  }

  std::string structure;

  if (const JsonValue* annotator = annotations.find("ImagesAnnotator");
      annotator != nullptr && annotator->read_text("version", structure)) {
    LOGD("The annotations are of the ImagesAnnotator structure " << structure);
  }

  // The folders of the relative paths are the ones below the folder of the
  // project file, so the project moves as a whole.
  std::error_code ecode;

  fs::path projectDir = fs::absolute(project, ecode).parent_path();

  if (ecode) {
    projectDir = project.parent_path();
  }

  for (const auto& folder : folders->get_items()) {
    assert(folder != nullptr);

    read_folder(*folder, projectDir, records);
  }

  measure_unsized(records);

  return true;
}

const std::string& Dxxwjz2pProject2DBImporter::format_name() const
{
  return dxxwjz2pFormat;
}

int Dxxwjz2pProject2DBImporter::format_version() const
{
  return dxxwjz2pVersion;
}

const std::string& Dxxwjz2pProject2DBImporter::project_extension() const
{
  return dxxwjz2pExtension;
}

fs::path Dxxwjz2pProject2DBImporter::locate_project(
    const std::filesystem::path& importPath) const
{
  if (fs::is_regular_file(importPath)) {
    return importPath;
  }

  const fs::path exported = importPath / (projectStem + project_extension());

  if (fs::is_regular_file(exported)) {
    return exported;
  }

  std::vector<fs::path> projects;

  try {
    for (const auto& entry : fs::directory_iterator{importPath}) {
      if (entry.is_regular_file() &&
          entry.path().extension() == project_extension()) {
        projects.emplace_back(entry.path());
      }
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the directory " << importPath.string()
                                       << " reason: " << e.what());
    return {};
  }

  if (projects.empty()) {
    return {};
  }

  // the same directory imports the same way twice
  std::sort(projects.begin(), projects.end());

  if (projects.size() > 1U) {
    LOGW("The directory " << importPath.string() << " holds " << projects.size()
                          << " " << project_extension()
                          << " projects, reading the first one by the name");
  }

  return projects.front();
}

void Dxxwjz2pProject2DBImporter::read_folder(
    const JsonValue& folder, const std::filesystem::path& projectDir,
    ImageRecordsSet& records) const
{
  const JsonValue* db = folder.find("db");

  std::string dbPath;

  if (db == nullptr || !db->read_text("path", dbPath) || dbPath.empty()) {
    LOGE("A folder of the project names no path of its images");
    return;
  }

  const JsonValue* images = folder.find(annotationsKey);

  if (images == nullptr || !images->is_array()) {
    LOGE("The folder " << dbPath << " of the project keeps no images");
    return;
  }

  fs::path folderPath{dbPath};

  if (folderPath.is_relative()) {
    folderPath = projectDir / folderPath;
  }

  folderPath = folderPath.lexically_normal();

  // the "images/" of a trailing separator is the "images" folder
  if (!folderPath.has_filename() && folderPath.has_parent_path() &&
      folderPath != folderPath.root_path()) {
    folderPath = folderPath.parent_path();
  }

  for (const auto& entry : images->get_items()) {
    assert(entry != nullptr);

    auto ir = read_image(*entry, folderPath);

    if (ir == nullptr) {
      continue;
    }

    records.emplace_back(ir);
  }
}

ImageRecordPtr Dxxwjz2pProject2DBImporter::read_image(
    const JsonValue& entry, const std::filesystem::path& folderPath)
{
  std::string relPath;

  if (!entry.read_text("rel_path", relPath) || relPath.empty()) {
    LOGE("An image of the folder " << folderPath.string() << " names no path");
    return {};
  }

  if (!fs::is_regular_file(folderPath / relPath)) {
    // The record is built all the same: the project keeps everything the
    // annotations of that image are made of, and a project whose pictures
    // were moved is repaired by pointing it at them again.
    LOGW("The project names an image which is not there: "
         << (folderPath / relPath).string());
  }

  // the folder and the path below it, the way the ImagesAnnotator project
  // keeps them
  auto ir = ImageRecord::create(relPath, folderPath.string());

  assert(ir != nullptr);

  double width{0.0};
  double height{0.0};

  // the 0 of the darknetxx is an unknown size
  if (entry.read_number("width", width) &&
      entry.read_number("height", height) && width > 0.0 && height > 0.0) {
    ir->iwidth = toPixels(width);
    ir->iheight = toPixels(height);
  }

  const JsonValue* boxes = entry.find(annotationsKey);

  if (boxes == nullptr || !boxes->is_array()) {
    LOGW("The image " << relPath << " keeps no boxes");
    return ir;
  }

  for (const auto& box : boxes->get_items()) {
    assert(box != nullptr);

    auto irr = read_box(*box);

    if (irr == nullptr) {
      LOGE("Skipping a box of the image " << relPath);
      continue;
    }

    ir->rects.emplace_back(irr);
  }

  return ir;
}

ImageRecordRectPtr Dxxwjz2pProject2DBImporter::read_box(const JsonValue& entry)
{
  std::string name;

  if (!entry.read_text("name", name) || name.empty()) {
    LOGE("A box of the project names no class");
    return {};
  }

  const JsonValue* rect = entry.find("rect");

  double x{0.0};
  double y{0.0};
  double width{0.0};
  double height{0.0};

  if (rect == nullptr || !rect->read_number("x", x) ||
      !rect->read_number("y", y) || !rect->read_number("width", width) ||
      !rect->read_number("height", height)) {
    LOGE("The box of the class " << name << " keeps no rect of four numbers");
    return {};
  }

  // The rect is the top left corner and the size in the pixels of the image,
  // the very four numbers of an ImageRecordRect, while the darknetxx writes
  // the floats of them.
  const int pwidth = toPixels(width);
  const int pheight = toPixels(height);

  if (pwidth <= 0 || pheight <= 0) {
    LOGE("The box of the class " << name << " is a box of no area");
    return {};
  }

  return std::make_shared<ImageRecordRect>(name, toPixels(x), toPixels(y),
                                           pwidth, pheight);
}

}  // namespace iannotator::importers
