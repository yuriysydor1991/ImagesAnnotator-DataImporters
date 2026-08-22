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

#include "src/importers/Ultralytics/UltralyticsDataYaml.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

#include "src/helpers/StringHelper.h"
#include "src/log/log.h"

namespace iannotator::importers
{

bool UltralyticsDataYaml::read(const std::vector<std::string>& lines)
{
  classNames.clear();
  root.clear();
  train.clear();
  val.clear();
  nextIndex = 0;

  bool inNames{false};

  for (const auto& raw : lines) {
    const std::string line = strip_comment(raw);
    const std::string entry = helpers::StringHelper::trim(line);

    if (entry.empty()) {
      continue;
    }

    // What belongs to the class names block is the indentation below it, and
    // the list items which an entry of the top level is written with as well.
    const bool nested =
        line.front() == ' ' || line.front() == '\t' || entry.front() == '-';

    if (nested && inNames) {
      read_name_entry(entry);
      continue;
    }

    inNames = read_entry(entry);
  }

  if (classNames.empty()) {
    LOGE("The descriptor declares no class name");
    return false;
  }

  return true;
}

const UltralyticsDataYaml::ClassNames& UltralyticsDataYaml::get_class_names()
    const
{
  return classNames;
}

const std::string& UltralyticsDataYaml::get_root() const { return root; }

const std::string& UltralyticsDataYaml::get_train() const { return train; }

const std::string& UltralyticsDataYaml::get_val() const { return val; }

bool UltralyticsDataYaml::read_entry(const std::string& line)
{
  const auto colon = line.find(':');

  if (colon == std::string::npos) {
    LOGT("Skipping the descriptor line which names no entry: " << line);
    return false;
  }

  const std::string key = helpers::StringHelper::trim(line.substr(0U, colon));
  const std::string value =
      helpers::StringHelper::trim(line.substr(colon + 1U));

  if (key == namesKey) {
    if (value.empty()) {
      // The block spelling: the names are the indented lines behind this one.
      return true;
    }

    read_names_list(value);

    return false;
  }

  if (key == rootKey) {
    root = unquote(value);
    return false;
  }

  if (key == trainKey) {
    train = unquote(value);
    return false;
  }

  if (key == valKey) {
    val = unquote(value);
    return false;
  }

  LOGT("Skipping the " << key << " entry, which describes no dataset");

  return false;
}

void UltralyticsDataYaml::read_name_entry(const std::string& line)
{
  if (line.front() == '-') {
    add_name(unquote(helpers::StringHelper::trim(line.substr(1U))));
    return;
  }

  const auto colon = line.find(':');

  if (colon == std::string::npos) {
    LOGE("The class names entry is neither an index nor a list item: " << line);
    return;
  }

  const std::string index = helpers::StringHelper::trim(line.substr(0U, colon));
  const std::string name =
      unquote(helpers::StringHelper::trim(line.substr(colon + 1U)));

  if (name.empty()) {
    LOGE("The class " << index << " names no annotation");
    return;
  }

  try {
    std::size_t consumed{0U};

    const long long read = std::stoll(index, &consumed);

    if (consumed != index.size() || read < 0) {
      LOGE("The class names entry carries no index: " << index);
      return;
    }

    classNames[read] = name;

    // A block of the explicit indices may be followed by a list spelling of
    // the very same entry, which then numbers its names behind them.
    nextIndex = std::max(nextIndex, read + 1);
  }
  catch (const std::exception& e) {
    LOGE("Fail to read the class index " << index << " reason: " << e.what());
  }
}

void UltralyticsDataYaml::read_names_list(const std::string& list)
{
  std::string body = list;

  if (!body.empty() && body.front() == '[') {
    body.erase(body.begin());
  }

  if (!body.empty() && body.back() == ']') {
    body.pop_back();
  }

  std::string item;
  char quote{'\0'};

  for (const char symbol : body) {
    if (quote != '\0') {
      if (symbol == quote) {
        quote = '\0';
      }

      item += symbol;

      continue;
    }

    if (symbol == '\'' || symbol == '"') {
      quote = symbol;
      item += symbol;
      continue;
    }

    // Only the commas standing outside a quoted name tell one from the next.
    if (symbol == ',') {
      add_name(unquote(helpers::StringHelper::trim(item)));
      item.clear();
      continue;
    }

    item += symbol;
  }

  const std::string last = helpers::StringHelper::trim(item);

  if (!last.empty()) {
    add_name(unquote(last));
  }
}

void UltralyticsDataYaml::add_name(const std::string& name)
{
  // The index of a name of either list spelling is its very position, so the
  // one which names nothing takes its place all the same.
  if (name.empty()) {
    LOGE("The class " << nextIndex << " names no annotation");
  } else {
    classNames[nextIndex] = name;
  }

  ++nextIndex;
}

std::string UltralyticsDataYaml::strip_comment(const std::string& line)
{
  char quote{'\0'};

  for (std::size_t iter = 0U; iter < line.size(); ++iter) {
    const char symbol = line[iter];

    if (quote != '\0') {
      if (symbol == quote) {
        quote = '\0';
      }

      continue;
    }

    if (symbol == '\'' || symbol == '"') {
      quote = symbol;
      continue;
    }

    if (symbol == '#') {
      return line.substr(0U, iter);
    }
  }

  return line;
}

std::string UltralyticsDataYaml::unquote(const std::string& value)
{
  constexpr const std::size_t quotesLength = 2U;

  if (value.size() < quotesLength) {
    return value;
  }

  const char quote = value.front();

  if ((quote != '\'' && quote != '"') || value.back() != quote) {
    return value;
  }

  const std::string body = value.substr(1U, value.size() - quotesLength);

  std::string read;

  read.reserve(body.size());

  for (std::size_t iter = 0U; iter < body.size(); ++iter) {
    const char symbol = body[iter];

    const bool doubledQuote = quote == '\'' && symbol == quote;
    const bool escape = quote == '"' && symbol == '\\';

    if ((doubledQuote || escape) && iter + 1U < body.size()) {
      read += body[iter + 1U];
      ++iter;
      continue;
    }

    read += symbol;
  }

  return read;
}

}  // namespace iannotator::importers
