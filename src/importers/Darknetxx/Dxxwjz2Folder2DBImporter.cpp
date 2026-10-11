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

#include "src/importers/Darknetxx/Dxxwjz2Folder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include "src/importers/Darknetxx/DxxwjzFileReader.h"
#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
namespace fs = std::filesystem;

const std::string dxxwjz2Format{"dxxwjz2"};
const std::string dxxwjz2Extension{".dxxwjz2"};
constexpr const int dxxwjz2Version{2};

/// @brief The network file the sibling exporters library writes, but its
/// extension
const std::string networkStem{"yolov4-obj"};
}  // namespace

Dxxwjz2Folder2DBImporter::ClassNames Dxxwjz2Folder2DBImporter::read_classes(
    const std::filesystem::path& importDir,
    const std::filesystem::path& namesPath)
{
  ClassNames names;

  if (fs::is_regular_file(namesPath)) {
    names = Yolo4Folder2DBImporter::read_classes(importDir, namesPath);
  }

  const fs::path network = locate_network(importDir);

  if (network.empty()) {
    LOGW("No " << network_extension() << " network found under "
               << (importDir / cfgRel).string()
               << ", reading the YOLO v4 directory it extends");
    return names;
  }

  ClassNames networkNames;

  if (!read_network(network, networkNames)) {
    LOGW("Fail to read the class names of the network " << network.string());
    return names;
  }

  if (names.empty()) {
    LOGI("No class names file found, reading the class names of the network "
         << network.string());
    return networkNames;
  }

  if (names != networkNames) {
    LOGW("The class names of "
         << namesPath.string() << " are not the ones of the network "
         << network.string() << ", the ones of the names file are read");
  }

  return names;
}

const std::string& Dxxwjz2Folder2DBImporter::format_name() const
{
  return dxxwjz2Format;
}

int Dxxwjz2Folder2DBImporter::format_version() const { return dxxwjz2Version; }

const std::string& Dxxwjz2Folder2DBImporter::network_extension() const
{
  return dxxwjz2Extension;
}

bool Dxxwjz2Folder2DBImporter::read_network_classes(
    const parsers::JsonValue& classes, ClassNames& names) const
{
  if (!classes.is_array()) {
    LOGE("The classes of the network are no array");
    return false;
  }

  const std::size_t count = classes.get_items().size();

  ClassNames indexed(count);
  std::vector<bool> named(count, false);

  // every index from zero once, in any order
  for (const auto& entry : classes.get_items()) {
    assert(entry != nullptr);

    double index{0.0};
    std::string name;

    if (!entry->read_number("index", index) ||
        !entry->read_text("name", name)) {
      LOGE("A class of the network carries no index and name pair");
      return false;
    }

    if (index < 0.0 || index >= static_cast<double>(count) ||
        std::floor(index) != index || named[static_cast<std::size_t>(index)]) {
      LOGE(
          "The class indexes of the network are not every index from zero "
          "once");
      return false;
    }

    indexed[static_cast<std::size_t>(index)] = name;
    named[static_cast<std::size_t>(index)] = true;
  }

  names = indexed;

  return true;
}

fs::path Dxxwjz2Folder2DBImporter::locate_network(
    const std::filesystem::path& importDir) const
{
  const fs::path cfgDir = importDir / cfgRel;
  const fs::path exported = cfgDir / (networkStem + network_extension());

  if (fs::is_regular_file(exported)) {
    return exported;
  }

  std::vector<fs::path> networks;

  try {
    if (!fs::is_directory(cfgDir)) {
      return {};
    }

    for (const auto& entry : fs::directory_iterator{cfgDir}) {
      if (entry.is_regular_file() &&
          entry.path().extension() == network_extension()) {
        networks.emplace_back(entry.path());
      }
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the directory " << cfgDir.string()
                                       << " reason: " << e.what());
    return {};
  }

  if (networks.empty()) {
    return {};
  }

  std::sort(networks.begin(), networks.end());

  return networks.front();
}

bool Dxxwjz2Folder2DBImporter::read_network(
    const std::filesystem::path& networkPath, ClassNames& names) const
{
  const DxxwjzFileReader::Path classesPath{"network", "classes"};

  DxxwjzFileReader::Members members;

  if (!DxxwjzFileReader{format_name(), format_version()}.read(
          networkPath, {classesPath}, members)) {
    return false;
  }

  const auto found = members.find(classesPath);

  if (found == members.end() || found->second == nullptr) {
    LOGE("The network " << networkPath.string() << " keeps no class names");
    return false;
  }

  return read_network_classes(*found->second, names);
}

}  // namespace iannotator::importers
