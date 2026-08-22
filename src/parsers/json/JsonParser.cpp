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

#include "src/parsers/json/JsonParser.h"

#include <cctype>
#include <cstddef>
#include <exception>
#include <memory>
#include <string>

#include "src/helpers/Utf8Helper.h"
#include "src/log/log.h"
#include "src/parsers/DepthGuard.h"

namespace iannotator::importers::parsers
{

namespace
{

/// @brief The halves a code point outside the basic plane is written as
constexpr const unsigned highSurrogateFirst = 0xD800U;
constexpr const unsigned highSurrogateLast = 0xDBFFU;
constexpr const unsigned lowSurrogateFirst = 0xDC00U;
constexpr const unsigned lowSurrogateLast = 0xDFFFU;

/// @brief The first code point which is no longer written as one code unit
constexpr const unsigned long supplementaryFirst = 0x10000UL;

/// @brief The lowest code point the format demands to be written as an escape
constexpr const unsigned char firstPrintable = 0x20U;

bool is_digit(const char& symbol)
{
  return std::isdigit(static_cast<unsigned char>(symbol)) != 0;
}

}  // namespace

JsonValuePtr JsonParser::parse(const std::string& document)
{
  input = &document;
  pos = 0U;
  depth = 0U;

  skip_blanks();

  JsonValuePtr root = parse_value();

  if (root != nullptr) {
    skip_blanks();

    if (!at_end()) {
      LOGE("Text left behind the JSON document at the offset " << pos);
      root.reset();
    }
  }

  // The document is not kept beyond the call: the very next parse of this
  // instance is handed a string of its own.
  input = nullptr;

  return root;
}

JsonValuePtr JsonParser::parse_value()
{
  if (depth >= maxDepth) {
    LOGE("The JSON document nests deeper than the " << maxDepth
                                                    << " levels read here");
    return {};
  }

  switch (current()) {
    case '{':
      return parse_object();
    case '[':
      return parse_array();
    case '"':
      return parse_string();
    case 't':
    case 'f':
    case 'n':
      return parse_keyword();
    default:
      break;
  }

  return parse_number();
}

JsonValuePtr JsonParser::parse_object()
{
  const DepthGuard guard{depth};

  if (!consume('{')) {
    return {};
  }

  auto value = std::make_unique<JsonValue>(JsonValue::Kind::Object);

  skip_blanks();

  if (consume('}')) {
    return value;
  }

  while (true) {
    skip_blanks();

    std::string key;

    if (!read_string(key)) {
      LOGE("No member name found at the offset " << pos);
      return {};
    }

    skip_blanks();

    if (!consume(':')) {
      LOGE("No colon behind the member name at the offset " << pos);
      return {};
    }

    skip_blanks();

    auto member = parse_value();

    if (member == nullptr) {
      return {};
    }

    value->add_member(key, std::move(member));

    skip_blanks();

    if (consume(',')) {
      continue;
    }

    if (consume('}')) {
      break;
    }

    LOGE("Neither a comma nor a closing brace at the offset " << pos);

    return {};
  }

  return value;
}

JsonValuePtr JsonParser::parse_array()
{
  const DepthGuard guard{depth};

  if (!consume('[')) {
    return {};
  }

  auto value = std::make_unique<JsonValue>(JsonValue::Kind::Array);

  skip_blanks();

  if (consume(']')) {
    return value;
  }

  while (true) {
    skip_blanks();

    auto item = parse_value();

    if (item == nullptr) {
      return {};
    }

    value->add_item(std::move(item));

    skip_blanks();

    if (consume(',')) {
      continue;
    }

    if (consume(']')) {
      break;
    }

    LOGE("Neither a comma nor a closing bracket at the offset " << pos);

    return {};
  }

  return value;
}

JsonValuePtr JsonParser::parse_string()
{
  std::string text;

  if (!read_string(text)) {
    return {};
  }

  auto value = std::make_unique<JsonValue>(JsonValue::Kind::String);

  value->set_text(text);

  return value;
}

JsonValuePtr JsonParser::parse_number()
{
  const std::size_t start = pos;

  if (current() == '-') {
    ++pos;
  }

  while (is_digit(current())) {
    ++pos;
  }

  if (current() == '.') {
    ++pos;

    while (is_digit(current())) {
      ++pos;
    }
  }

  if (current() == 'e' || current() == 'E') {
    ++pos;

    if (current() == '+' || current() == '-') {
      ++pos;
    }

    while (is_digit(current())) {
      ++pos;
    }
  }

  const std::string token = input->substr(start, pos - start);

  if (token.empty()) {
    LOGE("No JSON value found at the offset " << start);
    return {};
  }

  try {
    std::size_t consumed{0U};

    const double read = std::stod(token, &consumed);

    if (consumed != token.size()) {
      LOGE("The number " << token << " at the offset " << start
                         << " is no JSON one");
      return {};
    }

    auto value = std::make_unique<JsonValue>(JsonValue::Kind::Number);

    value->set_number(read);

    return value;
  }
  catch (const std::exception& e) {
    LOGE("Fail to read the number " << token << " at the offset " << start
                                    << " reason: " << e.what());
  }

  return {};
}

JsonValuePtr JsonParser::parse_keyword()
{
  static const std::string trueWord{"true"};
  static const std::string falseWord{"false"};
  static const std::string nullWord{"null"};

  if (input->compare(pos, trueWord.size(), trueWord) == 0) {
    pos += trueWord.size();

    auto value = std::make_unique<JsonValue>(JsonValue::Kind::Boolean);

    value->set_number(1.0);

    return value;
  }

  if (input->compare(pos, falseWord.size(), falseWord) == 0) {
    pos += falseWord.size();

    return std::make_unique<JsonValue>(JsonValue::Kind::Boolean);
  }

  if (input->compare(pos, nullWord.size(), nullWord) == 0) {
    pos += nullWord.size();

    return std::make_unique<JsonValue>(JsonValue::Kind::Null);
  }

  LOGE("No JSON keyword found at the offset " << pos);

  return {};
}

bool JsonParser::read_string(std::string& value)
{
  if (!consume('"')) {
    return false;
  }

  std::string read;

  while (!at_end()) {
    const char symbol = current();

    if (symbol == '"') {
      ++pos;

      value.swap(read);

      return true;
    }

    if (symbol == '\\') {
      ++pos;

      if (!read_escape(read)) {
        return false;
      }

      continue;
    }

    if (static_cast<unsigned char>(symbol) < firstPrintable) {
      LOGE("A raw control character inside the string at the offset " << pos);
      return false;
    }

    read += symbol;

    ++pos;
  }

  LOGE("No closing quote found for the string");

  return false;
}

bool JsonParser::read_escape(std::string& value)
{
  if (at_end()) {
    LOGE("The document ends behind a string escape");
    return false;
  }

  const char symbol = current();

  ++pos;

  switch (symbol) {
    case '"':
    case '\\':
    case '/':
      value += symbol;
      return true;
    case 'b':
      value += '\b';
      return true;
    case 'f':
      value += '\f';
      return true;
    case 'n':
      value += '\n';
      return true;
    case 'r':
      value += '\r';
      return true;
    case 't':
      value += '\t';
      return true;
    case 'u':
      break;
    default:
      LOGE("An unknown string escape at the offset " << pos);
      return false;
  }

  unsigned code{0U};

  if (!read_code_unit(code)) {
    return false;
  }

  if (code >= lowSurrogateFirst && code <= lowSurrogateLast) {
    LOGE("A low surrogate without its high half at the offset " << pos);
    return false;
  }

  unsigned long codepoint = code;

  // A code point outside the basic plane is written as the two halves of a
  // surrogate pair, which name a character together and none apart.
  if (code >= highSurrogateFirst && code <= highSurrogateLast) {
    unsigned low{0U};

    if (!consume('\\') || !consume('u') || !read_code_unit(low) ||
        low < lowSurrogateFirst || low > lowSurrogateLast) {
      LOGE("A high surrogate without its low half at the offset " << pos);
      return false;
    }

    codepoint = supplementaryFirst + ((codepoint - highSurrogateFirst) << 10U) +
                (low - lowSurrogateFirst);
  }

  helpers::Utf8Helper::append(codepoint, value);

  return true;
}

bool JsonParser::read_code_unit(unsigned& code)
{
  constexpr const std::size_t hexDigits = 4U;
  constexpr const unsigned hexBase = 16U;

  unsigned read{0U};

  for (std::size_t iter = 0U; iter < hexDigits; ++iter) {
    const char symbol = current();

    unsigned digit{0U};

    if (symbol >= '0' && symbol <= '9') {
      digit = static_cast<unsigned>(symbol - '0');
    } else if (symbol >= 'a' && symbol <= 'f') {
      digit = static_cast<unsigned>(symbol - 'a') + 10U;
    } else if (symbol >= 'A' && symbol <= 'F') {
      digit = static_cast<unsigned>(symbol - 'A') + 10U;
    } else {
      LOGE("No hexadecimal digit at the offset " << pos);
      return false;
    }

    read = (read * hexBase) + digit;

    ++pos;
  }

  code = read;

  return true;
}

void JsonParser::skip_blanks()
{
  while (!at_end()) {
    const char symbol = current();

    if (symbol != ' ' && symbol != '\t' && symbol != '\n' && symbol != '\r') {
      return;
    }

    ++pos;
  }
}

bool JsonParser::at_end() const
{
  return input == nullptr || pos >= input->size();
}

char JsonParser::current() const { return at_end() ? '\0' : (*input)[pos]; }

bool JsonParser::consume(const char& expected)
{
  if (current() != expected) {
    return false;
  }

  ++pos;

  return true;
}

JsonParserPtr JsonParser::create() { return std::make_shared<JsonParser>(); }

}  // namespace iannotator::importers::parsers
