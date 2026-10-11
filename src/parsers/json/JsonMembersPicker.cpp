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

#include "src/parsers/json/JsonMembersPicker.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "src/log/log.h"
#include "src/parsers/DepthGuard.h"
#include "src/parsers/json/JsonParser.h"

namespace iannotator::importers::parsers
{

namespace
{
constexpr const int endOfDocument = -1;

bool is_blank(int symbol)
{
  return symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r';
}
}  // namespace

JsonMembersPicker::JsonMembersPicker(Source nsource)
    : source{std::move(nsource)}
{
}

bool JsonMembersPicker::pick(const std::vector<Path>& wanted,
                             const Receiver& nreceiver)
{
  buffer.resize(bufferSize);
  pos = 0U;
  size = 0U;
  ended = false;
  depth = 0U;
  pending = wanted;
  receiver = &nreceiver;
  finished = pending.empty();

  if (finished) {
    return true;
  }

  skip_blanks();

  if (peek() != '{') {
    LOGE("The document is no JSON object");
    return false;
  }

  Path path;

  if (!read_object(path)) {
    return false;
  }

  if (finished) {
    return true;
  }

  skip_blanks();

  if (peek() != endOfDocument) {
    LOGE("The document holds more than its one JSON value");
    return false;
  }

  return true;
}

bool JsonMembersPicker::read_object(Path& path)
{
  if (depth >= maxDepth) {
    LOGE("The objects of the document nest deeper than " << maxDepth
                                                         << " levels");
    return false;
  }

  const DepthGuard guard{depth};

  consume('{');
  skip_blanks();

  if (consume('}')) {
    return true;
  }

  for (;;) {
    skip_blanks();

    std::string key;

    if (!read_key(key)) {
      return false;
    }

    skip_blanks();

    if (!consume(':')) {
      LOGE("The member " << key << " of the document is followed by no colon");
      return false;
    }

    path.emplace_back(std::move(key));

    const bool read = read_member_value(path);

    path.pop_back();

    if (!read) {
      return false;
    }

    if (finished) {
      return true;
    }

    skip_blanks();

    if (consume(',')) {
      continue;
    }

    if (consume('}')) {
      return true;
    }

    LOGE(
        "A member of the document is followed by neither a comma nor the "
        "end of its object");
    return false;
  }
}

bool JsonMembersPicker::read_member_value(Path& path)
{
  skip_blanks();

  if (is_pending(path)) {
    pending.erase(std::find(pending.begin(), pending.end(), path));

    std::string text;

    if (!skip_value(&text)) {
      return false;
    }

    if (!(*receiver)(path, text) || pending.empty()) {
      finished = true;
    }

    return true;
  }

  if (peek() == '{' && leads_to_pending(path)) {
    return read_object(path);
  }

  return skip_value(nullptr);
}

bool JsonMembersPicker::read_key(std::string& key)
{
  if (peek() != '"') {
    LOGE("A member of the document is named by no string");
    return false;
  }

  std::string raw;

  if (!skip_string(&raw)) {
    return false;
  }

  // The keys of the darknetxx documents are plain: the parse of the escapes
  // is left to the JsonParser, of the few keys which hold any.
  if (raw.find('\\') == std::string::npos) {
    key = raw.substr(1U, raw.size() - 2U);
    return true;
  }

  JsonParser parser;

  const JsonValuePtr value = parser.parse(raw);

  if (value == nullptr || !value->is_string()) {
    LOGE("A member of the document is named by a malformed string");
    return false;
  }

  key = value->get_text();

  return true;
}

bool JsonMembersPicker::skip_value(std::string* text)
{
  skip_blanks();

  const int symbol = peek();

  if (symbol == '{' || symbol == '[') {
    return skip_container(text);
  }

  if (symbol == '"') {
    return skip_string(text);
  }

  if (symbol == endOfDocument || symbol == ',' || symbol == ':' ||
      symbol == '}' || symbol == ']') {
    LOGE("The document holds no value where a value is expected");
    return false;
  }

  return skip_scalar(text);
}

bool JsonMembersPicker::skip_container(std::string* text)
{
  // the brackets opened and not closed yet, the innermost last
  std::string opened;
  bool inString{false};
  bool escaped{false};

  for (;;) {
    if (pos == size && !fill()) {
      LOGE("The document ends inside of a value");
      return false;
    }

    const std::size_t start = pos;
    bool closed{false};

    while (pos < size && !closed) {
      const char symbol = buffer[pos++];

      if (inString) {
        if (escaped) {
          escaped = false;
        } else if (symbol == '\\') {
          escaped = true;
        } else if (symbol == '"') {
          inString = false;
        }
        continue;
      }

      switch (symbol) {
        case '"':
          inString = true;
          break;
        case '{':
        case '[':
          if (opened.size() >= maxDepth) {
            LOGE("The values of the document nest deeper than " << maxDepth
                                                                << " levels");
            return false;
          }
          opened.push_back(symbol);
          break;
        case '}':
        case ']':
          if (opened.empty() || opened.back() != (symbol == '}' ? '{' : '[')) {
            LOGE("A bracket of the document closes no bracket of its kind");
            return false;
          }
          opened.pop_back();
          closed = opened.empty();
          break;
        default:
          break;
      }
    }

    if (text != nullptr) {
      text->append(buffer.data() + start, pos - start);
    }

    if (closed) {
      return true;
    }
  }
}

bool JsonMembersPicker::skip_string(std::string* text)
{
  bool opened{false};
  bool escaped{false};

  for (;;) {
    if (pos == size && !fill()) {
      LOGE("The document ends inside of a string");
      return false;
    }

    const std::size_t start = pos;
    bool closed{false};

    while (pos < size && !closed) {
      const char symbol = buffer[pos++];

      if (!opened) {
        // the opening quote itself, the caller has peeked at
        opened = true;
      } else if (escaped) {
        escaped = false;
      } else if (symbol == '\\') {
        escaped = true;
      } else if (symbol == '"') {
        closed = true;
      }
    }

    if (text != nullptr) {
      text->append(buffer.data() + start, pos - start);
    }

    if (closed) {
      return true;
    }
  }
}

bool JsonMembersPicker::skip_scalar(std::string* text)
{
  for (;;) {
    const int symbol = peek();

    if (symbol == endOfDocument || is_blank(symbol) || symbol == ',' ||
        symbol == '}' || symbol == ']') {
      return true;
    }

    if (text != nullptr) {
      text->push_back(static_cast<char>(symbol));
    }

    ++pos;
  }
}

bool JsonMembersPicker::is_pending(const Path& path) const
{
  return std::find(pending.cbegin(), pending.cend(), path) != pending.cend();
}

bool JsonMembersPicker::leads_to_pending(const Path& path) const
{
  return std::any_of(pending.cbegin(), pending.cend(), [&path](const Path& p) {
    return p.size() > path.size() &&
           std::equal(path.cbegin(), path.cend(), p.cbegin());
  });
}

void JsonMembersPicker::skip_blanks()
{
  while (is_blank(peek())) {
    ++pos;
  }
}

int JsonMembersPicker::peek()
{
  if (pos == size && !fill()) {
    return endOfDocument;
  }

  return static_cast<unsigned char>(buffer[pos]);
}

bool JsonMembersPicker::consume(char expected)
{
  if (peek() != static_cast<unsigned char>(expected)) {
    return false;
  }

  ++pos;

  return true;
}

bool JsonMembersPicker::fill()
{
  if (ended) {
    return false;
  }

  pos = 0U;
  size = source ? source(buffer.data(), buffer.size()) : 0U;

  if (size == 0U) {
    ended = true;
    return false;
  }

  return true;
}

}  // namespace iannotator::importers::parsers
