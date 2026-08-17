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

#include "src/importers/PlainTxt/PlainTxtFolder2DBImporter.h"

#include <algorithm>
#include <cassert>
#include <cctype>
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

/// @brief How many numbers one rectangle takes on a line: x, y, width, height
constexpr const std::size_t rectFields = 4U;

/**
 * @brief Reads a whole token as a non negative integer.
 *
 * @param token The token to read.
 * @param value Out: the read value, left alone on a failure.
 *
 * @return Returns true when the token was a number and nothing else.
 */
bool to_unsigned(const std::string& token, unsigned long long& value)
{
  if (token.empty() ||
      !std::all_of(token.cbegin(), token.cend(),
                   [](unsigned char c) { return std::isdigit(c) != 0; })) {
    return false;
  }

  try {
    value = std::stoull(token);
  }
  catch (const std::exception& e) {
    LOGE("Fail to read the number " << token << " reason: " << e.what());
    return false;
  }

  return true;
}

/**
 * @brief Reads a whole token as an integer coordinate.
 *
 * @param token The token to read.
 * @param value Out: the read value, left alone on a failure.
 *
 * @return Returns true when the token was a number and nothing else.
 */
bool to_int(const std::string& token, int& value)
{
  try {
    std::size_t consumed{0U};

    const int read = std::stoi(token, &consumed);

    if (consumed != token.size()) {
      return false;
    }

    value = read;
  }
  catch (const std::exception& e) {
    LOGT("Fail to read the coordinate " << token << " reason: " << e.what());
    return false;
  }

  return true;
}

}  // namespace

const std::string PlainTxtFolder2DBImporter::txtExt = ".txt";

bool PlainTxtFolder2DBImporter::read_dataset(
    const std::filesystem::path& importDir, ImageRecordsSet& outRecords)
{
  path2record.clear();
  records.clear();

  std::vector<fs::path> tagFiles;

  try {
    for (const auto& entry : fs::directory_iterator{importDir}) {
      if (!entry.is_regular_file() || entry.path().extension() != txtExt) {
        continue;
      }

      tagFiles.emplace_back(entry.path());
    }
  }
  catch (const std::exception& e) {
    LOGE("Fail to walk the import directory " << importDir.string()
                                              << " reason: " << e.what());
    return false;
  }

  // The filesystem hands the entries over in no particular order, while the
  // annotation names decide nothing but themselves here. Sorting only keeps
  // one and the same directory importing one and the same way twice.
  std::sort(tagFiles.begin(), tagFiles.end());

  for (const auto& fpath : tagFiles) {
    read_tag_file(fpath, fpath.stem().string());
  }

  // The layout carries no image size of its own, so this is filled in only
  // when there is something to measure the pictures with. A record left with
  // zeroes still holds every rectangle: they are stored in the image own
  // pixels here, which is what the internal format keeps them in as well.
  if (has_image_sizer()) {
    for (auto& ir : records) {
      int width{0};
      int height{0};

      if (measure_image(ir->get_full_path(), width, height)) {
        ir->iwidth = width;
        ir->iheight = height;
      }
    }
  }

  outRecords.swap(records);

  return true;
}

void PlainTxtFolder2DBImporter::read_tag_file(
    const std::filesystem::path& fpath, const std::string& tagName)
{
  if (tagName.empty()) {
    LOGE("The file names no annotation: " << fpath.string());
    return;
  }

  LOGT("Reading the annotation " << tagName << " out of " << fpath.string());

  for (const auto& line : read_lines(fpath)) {
    read_tag_line(line, tagName);
  }
}

void PlainTxtFolder2DBImporter::read_tag_line(const std::string& line,
                                              const std::string& tagName)
{
  const Tokens tokens = tokenize(line);

  const Tokens::size_type countIndex = find_count_token(tokens);

  if (countIndex >= tokens.size()) {
    LOGE("Fail to read the line: " << line);
    return;
  }

  std::string imagePath = tokens.front();

  // Everything before the count is the image path, which the export writes
  // unquoted, so a path holding a space arrives split.
  for (Tokens::size_type iter = 1U; iter < countIndex; ++iter) {
    imagePath += " " + tokens[iter];
  }

  auto ir = get_record(imagePath);

  assert(ir != nullptr);

  for (Tokens::size_type iter = countIndex + 1U; iter < tokens.size();
       iter += rectFields) {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    if (!to_int(tokens[iter], x) || !to_int(tokens[iter + 1U], y) ||
        !to_int(tokens[iter + 2U], width) ||
        !to_int(tokens[iter + 3U], height)) {
      LOGE("Fail to read the rectangle " << tagName
                                         << " of the line: " << line);
      continue;
    }

    ir->rects.emplace_back(
        std::make_shared<ImageRecordRect>(tagName, x, y, width, height));
  }
}

ImageRecordPtr PlainTxtFolder2DBImporter::get_record(
    const std::string& imagePath)
{
  auto miter = path2record.find(imagePath);

  if (miter != path2record.end()) {
    LOGT("cache hit");
    return miter->second;
  }

  auto ir = create_record(imagePath);

  path2record[imagePath] = ir;
  records.emplace_back(ir);

  return ir;
}

PlainTxtFolder2DBImporter::Tokens PlainTxtFolder2DBImporter::tokenize(
    const std::string& line)
{
  Tokens tokens;

  std::istringstream iss{line};
  std::string token;

  while (iss >> token) {
    tokens.emplace_back(token);
  }

  return tokens;
}

PlainTxtFolder2DBImporter::Tokens::size_type
PlainTxtFolder2DBImporter::find_count_token(const Tokens& tokens)
{
  // The first token is the image path itself, so the count can only follow it.
  for (Tokens::size_type iter = 1U; iter + 1U <= tokens.size(); ++iter) {
    unsigned long long count{0U};

    if (!to_unsigned(tokens[iter], count)) {
      continue;
    }

    const Tokens::size_type behind = tokens.size() - iter - 1U;

    if (behind % rectFields == 0U && behind / rectFields == count) {
      return iter;
    }
  }

  return tokens.size();
}

}  // namespace iannotator::importers
