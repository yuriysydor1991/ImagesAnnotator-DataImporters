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

#include "src/importers/Darknetxx/DxxwjzFileReader.h"

#include <cstddef>
#include <filesystem>
#include <ios>
#include <memory>
#include <string>
#include <utility>

#include "src/log/log.h"
#include "src/parsers/json/JsonParser.h"
#include "src/zlib/GzipFileStreams.h"
#include "src/zlib/ZlibController.h"

namespace iannotator::importers
{

DxxwjzFileReader::DxxwjzFileReader(std::string nformat, int nversion)
    : format{std::move(nformat)}, version{nversion}
{
}

bool DxxwjzFileReader::read(const std::filesystem::path& fpath,
                            const Paths& wanted, Members& members) const
{
  std::unique_ptr<iadi0zlibi::GzipIStream> file =
      iadi0zlibi::ZlibController::create()->create_file_reader(fpath.string());

  if (file == nullptr) {
    LOGE("Fail to open the " << format << " file " << fpath.string());
    return false;
  }

  if (file->direct()) {
    LOGW("The " << format << " file " << fpath.string()
                << " is no gzip one, reading its plain JSON text");
  }

  parsers::JsonMembersPicker picker{
      [&file](char* buffer, std::size_t size) -> std::size_t {
        file->read(buffer, static_cast<std::streamsize>(size));
        return static_cast<std::size_t>(file->gcount());
      }};

  Paths all{wanted};

  all.push_back(Path{formatKey});
  all.push_back(Path{versionKey});

  bool formatRead{false};
  bool versionRead{false};
  bool refused{false};

  parsers::JsonParser parser;

  const bool picked =
      picker.pick(all, [&](const Path& path, const std::string& text) {
        parsers::JsonValuePtr value = parser.parse(text);

        if (value == nullptr) {
          LOGE("The " << join(path) << " member of " << fpath.string()
                      << " is a malformed JSON value");
          refused = true;
          return false;
        }

        if (path == Path{formatKey}) {
          formatRead = true;

          if (!value->is_string() || value->get_text() != format) {
            LOGE("The file " << fpath.string() << " is the " << text
                             << " document, not the " << format << " one");
            refused = true;
            return false;
          }

          return true;
        }

        if (path == Path{versionKey}) {
          versionRead = true;

          // the darknetxx readers refuse every version but their own
          if (!value->is_number() ||
              value->get_number() != static_cast<double>(version)) {
            LOGE("The file " << fpath.string() << " is of the " << format
                             << " version " << text << ", while the version "
                             << version << " alone is read");
            refused = true;
            return false;
          }

          return true;
        }

        members[path] = std::move(value);

        return true;
      });

  if (file->failed()) {
    LOGE("Fail to decompress the " << format << " file " << fpath.string());
    return false;
  }

  if (refused) {
    return false;
  }

  if (!picked) {
    LOGE("The file " << fpath.string()
                     << " holds no well formed JSON document");
    return false;
  }

  if (!formatRead || !versionRead) {
    LOGE("The file " << fpath.string()
                     << " names no format and version: this is no " << format
                     << " file");
    return false;
  }

  return true;
}

std::string DxxwjzFileReader::join(const Path& path)
{
  std::string joined;

  for (const auto& key : path) {
    joined += (joined.empty() ? "" : ".") + key;
  }

  return joined;
}

}  // namespace iannotator::importers
