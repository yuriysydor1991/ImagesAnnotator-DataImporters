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

#include "src/importers/Darknetxx/Dxxwjz3Folder2DBImporter.h"

#include <cassert>
#include <string>

#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
const std::string dxxwjz3Format{"dxxwjz3"};
const std::string dxxwjz3Extension{".dxxwjz3"};
constexpr const int dxxwjz3Version{3};
}  // namespace

const std::string& Dxxwjz3Folder2DBImporter::format_name() const
{
  return dxxwjz3Format;
}

int Dxxwjz3Folder2DBImporter::format_version() const { return dxxwjz3Version; }

const std::string& Dxxwjz3Folder2DBImporter::network_extension() const
{
  return dxxwjz3Extension;
}

bool Dxxwjz3Folder2DBImporter::read_network_classes(
    const parsers::JsonValue& classes, ClassNames& names) const
{
  if (!classes.is_array()) {
    LOGE("The classes of the network are no array");
    return false;
  }

  ClassNames read;

  // the name of an index is the one of the class of the index
  for (const auto& entry : classes.get_items()) {
    assert(entry != nullptr);

    if (!entry->is_string()) {
      LOGE("A class of the network is named by no string");
      return false;
    }

    read.emplace_back(entry->get_text());
  }

  names = read;

  return true;
}

}  // namespace iannotator::importers
