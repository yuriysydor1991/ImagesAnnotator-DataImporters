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

#include "src/importers/JsonDescriptor2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <filesystem>
#include <string>

#include "src/log/log.h"
#include "src/parsers/json/JsonParser.h"

namespace iannotator::importers
{

namespace
{

namespace fs = std::filesystem;

const std::string jsonExt = ".json";

}  // namespace

JsonDescriptor2DBImporter::JsonValuePtr
JsonDescriptor2DBImporter::read_descriptor(
    const Paths& candidates, std::filesystem::path& descriptorPath) const
{
  auto parser = parsers::JsonParser::create();

  assert(parser != nullptr);

  for (const auto& candidate : candidates) {
    std::string document;

    if (!read_file(candidate, document)) {
      continue;
    }

    JsonValuePtr root = parser->parse(document);

    if (root == nullptr) {
      LOGD("No JSON document read out of " << candidate.string());
      continue;
    }

    if (!holds_dataset(*root)) {
      LOGD("The JSON file " << candidate.string()
                            << " describes no dataset of the wanted layout");
      continue;
    }

    descriptorPath = candidate;

    return root;
  }

  return {};
}

void JsonDescriptor2DBImporter::collect_descriptors(
    const std::filesystem::path& dirPath, Paths& candidates)
{
  if (!fs::is_directory(dirPath)) {
    return;
  }

  Paths found;

  try {
    for (const auto& entry : fs::directory_iterator{dirPath}) {
      if (!entry.is_regular_file() || entry.path().extension() != jsonExt) {
        continue;
      }

      // The name the export writes is the first candidate of all, and it is
      // no second one.
      if (std::find(candidates.cbegin(), candidates.cend(), entry.path()) ==
          candidates.cend()) {
        found.emplace_back(entry.path());
      }
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the directory " << dirPath.string()
                                       << " reason: " << e.what());
    return;
  }

  // The filesystem hands the entries over in no particular order, so this only
  // keeps one and the same directory importing one and the same way twice.
  std::sort(found.begin(), found.end());

  candidates.insert(candidates.end(), found.cbegin(), found.cend());
}

}  // namespace iannotator::importers
