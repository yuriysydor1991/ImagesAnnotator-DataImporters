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

#include "src/parsers/xml/XmlParser.h"

#include <cctype>
#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <utility>

#include "src/helpers/Utf8Helper.h"
#include "src/log/log.h"
#include "src/parsers/DepthGuard.h"

namespace iannotator::importers::parsers
{

namespace
{

const std::string commentOpening{"<!--"};
const std::string commentClosing{"-->"};
const std::string instructionOpening{"<?"};
const std::string instructionClosing{"?>"};
const std::string doctypeOpening{"<!DOCTYPE"};
const std::string cdataOpening{"<![CDATA["};
const std::string cdataClosing{"]]>"};
const std::string endTagOpening{"</"};

/// @brief The highest code point a numeric character reference may name
constexpr const unsigned long lastCodepoint = 0x10FFFFUL;

bool is_name_symbol(const char& symbol)
{
  const auto value = static_cast<unsigned char>(symbol);

  // Everything outside ASCII is a name symbol: an element name of a document
  // written in one more alphabet arrives here as its UTF-8 bytes.
  return std::isalnum(value) != 0 || symbol == '_' || symbol == '-' ||
         symbol == '.' || symbol == ':' || value >= 0x80U;
}

}  // namespace

XmlNodePtr XmlParser::parse(const std::string& document)
{
  input = &document;
  pos = 0U;
  depth = 0U;

  XmlNodePtr root;

  if (skip_prologue()) {
    root = parse_element();
  }

  if (root != nullptr && (!skip_prologue() || !at_end())) {
    LOGE("Text left behind the XML root element at the offset " << pos);
    root.reset();
  }

  // The document is not kept beyond the call: the very next parse of this
  // instance is handed a string of its own.
  input = nullptr;

  return root;
}

XmlNodePtr XmlParser::parse_element()
{
  if (depth >= maxDepth) {
    LOGE("The XML document nests deeper than the " << maxDepth
                                                   << " levels read here");
    return {};
  }

  const DepthGuard guard{depth};

  if (!consume('<')) {
    LOGE("No element found at the offset " << pos);
    return {};
  }

  std::string name;

  if (!read_name(name)) {
    LOGE("No element name found at the offset " << pos);
    return {};
  }

  bool selfClosing{false};

  if (!read_attributes(selfClosing)) {
    return {};
  }

  auto element = std::make_unique<XmlNode>(name);

  if (selfClosing) {
    return element;
  }

  if (!read_content(*element)) {
    return {};
  }

  pos += endTagOpening.size();

  std::string closingName;

  if (!read_name(closingName) || closingName != name) {
    LOGE("The " << name << " element is closed by the " << closingName
                << " end tag at the offset " << pos);
    return {};
  }

  skip_blanks();

  if (!consume('>')) {
    LOGE("The end tag of the " << name << " element is left open at the offset "
                               << pos);
    return {};
  }

  return element;
}

bool XmlParser::read_content(XmlNode& element)
{
  std::string text;

  while (!at_end()) {
    if (starts_with(endTagOpening)) {
      element.append_text(text);
      return true;
    }

    if (starts_with(commentOpening)) {
      if (!skip_span(commentOpening, commentClosing)) {
        return false;
      }

      continue;
    }

    if (starts_with(cdataOpening)) {
      if (!read_cdata(text)) {
        return false;
      }

      continue;
    }

    if (starts_with(instructionOpening)) {
      if (!skip_span(instructionOpening, instructionClosing)) {
        return false;
      }

      continue;
    }

    if (current() == '<') {
      element.append_text(text);

      text.clear();

      auto child = parse_element();

      if (child == nullptr) {
        return false;
      }

      element.add_child(std::move(child));

      continue;
    }

    if (current() == '&') {
      read_reference(text);

      continue;
    }

    text += current();

    ++pos;
  }

  LOGE("No end tag found for the " << element.get_name() << " element");

  return false;
}

bool XmlParser::read_name(std::string& name)
{
  const std::size_t start = pos;

  while (!at_end() && is_name_symbol(current())) {
    ++pos;
  }

  if (pos == start) {
    return false;
  }

  name = input->substr(start, pos - start);

  return true;
}

bool XmlParser::read_attributes(bool& selfClosing)
{
  static const std::string selfClosingTail{"/>"};

  selfClosing = false;

  while (true) {
    skip_blanks();

    if (starts_with(selfClosingTail)) {
      pos += selfClosingTail.size();

      selfClosing = true;

      return true;
    }

    if (consume('>')) {
      return true;
    }

    std::string name;

    if (!read_name(name)) {
      LOGE("No attribute name found at the offset " << pos);
      return false;
    }

    skip_blanks();

    if (!consume('=')) {
      LOGE("No equals sign behind the attribute " << name << " at the offset "
                                                  << pos);
      return false;
    }

    skip_blanks();

    const char quote = current();

    if (quote != '"' && quote != '\'') {
      LOGE("The attribute " << name << " carries no quoted value at the offset "
                            << pos);
      return false;
    }

    ++pos;

    const auto end = input->find(quote, pos);

    if (end == std::string::npos) {
      LOGE("No closing quote found for the attribute " << name);
      return false;
    }

    pos = end + 1U;
  }
}

void XmlParser::read_reference(std::string& value)
{
  const auto end = input->find(';', pos + 1U);
  const std::size_t bodyLength = end == std::string::npos ? 0U : end - pos - 1U;

  if (bodyLength == 0U || bodyLength > maxReferenceLength) {
    LOGT("A stray ampersand in the text at the offset " << pos);

    value += current();

    ++pos;

    return;
  }

  const std::string body = input->substr(pos + 1U, bodyLength);

  pos = end + 1U;

  if (body == "amp") {
    value += '&';
    return;
  }

  if (body == "lt") {
    value += '<';
    return;
  }

  if (body == "gt") {
    value += '>';
    return;
  }

  if (body == "quot") {
    value += '"';
    return;
  }

  if (body == "apos") {
    value += '\'';
    return;
  }

  if (body.front() == '#' && read_numeric_reference(body, value)) {
    return;
  }

  // The entity may well be defined in a document type declaration this reader
  // reads over, so the reference is left in the text as it stands instead of
  // costing the whole descriptor its import.
  LOGW("The entity reference &" << body
                                << "; is left in the text as it stands");

  value += '&';
  value += body;
  value += ';';
}

bool XmlParser::read_numeric_reference(const std::string& body,
                                       std::string& value)
{
  static const std::string decimalDigits{"0123456789"};
  static const std::string hexDigits{"0123456789abcdefABCDEF"};

  const bool hex = body.size() > 1U && (body[1U] == 'x' || body[1U] == 'X');

  const std::string digits = body.substr(hex ? 2U : 1U);

  if (digits.empty() ||
      digits.find_first_not_of(hex ? hexDigits : decimalDigits) !=
          std::string::npos) {
    return false;
  }

  try {
    constexpr const int hexBase = 16;
    constexpr const int decimalBase = 10;

    const unsigned long codepoint =
        std::stoul(digits, nullptr, hex ? hexBase : decimalBase);

    if (codepoint > lastCodepoint) {
      return false;
    }

    helpers::Utf8Helper::append(codepoint, value);
  }
  catch (const std::exception& e) {
    LOGE("Fail to read the character reference &" << body
                                                  << "; reason: " << e.what());
    return false;
  }

  return true;
}

bool XmlParser::read_cdata(std::string& value)
{
  const std::size_t start = pos + cdataOpening.size();

  const auto end = input->find(cdataClosing, start);

  if (end == std::string::npos) {
    LOGE("No " << cdataClosing << " found for the CDATA section at the offset "
               << pos);
    return false;
  }

  value.append(*input, start, end - start);

  pos = end + cdataClosing.size();

  return true;
}

bool XmlParser::skip_prologue()
{
  while (true) {
    skip_blanks();

    if (starts_with(commentOpening)) {
      if (!skip_span(commentOpening, commentClosing)) {
        return false;
      }

      continue;
    }

    if (starts_with(instructionOpening)) {
      if (!skip_span(instructionOpening, instructionClosing)) {
        return false;
      }

      continue;
    }

    if (starts_with(doctypeOpening)) {
      if (!skip_doctype()) {
        return false;
      }

      continue;
    }

    return true;
  }
}

bool XmlParser::skip_span(const std::string& opening,
                          const std::string& closing)
{
  const std::size_t start = pos;

  pos += opening.size();

  const auto end = input->find(closing, pos);

  if (end == std::string::npos) {
    LOGE("No " << closing << " found for the " << opening << " at the offset "
               << start);
    return false;
  }

  pos = end + closing.size();

  return true;
}

bool XmlParser::skip_doctype()
{
  const std::size_t start = pos;

  pos += doctypeOpening.size();

  while (!at_end()) {
    if (consume('>')) {
      return true;
    }

    // The internal subset holds angle brackets of its own, so it is read over
    // as a whole instead of symbol by symbol.
    if (current() == '[') {
      const auto end = input->find(']', pos);

      if (end == std::string::npos) {
        break;
      }

      pos = end + 1U;

      continue;
    }

    ++pos;
  }

  LOGE("The document type declaration at the offset " << start
                                                      << " is left open");

  return false;
}

void XmlParser::skip_blanks()
{
  while (!at_end()) {
    const char symbol = current();

    if (symbol != ' ' && symbol != '\t' && symbol != '\n' && symbol != '\r') {
      return;
    }

    ++pos;
  }
}

bool XmlParser::at_end() const
{
  return input == nullptr || pos >= input->size();
}

char XmlParser::current() const { return at_end() ? '\0' : (*input)[pos]; }

bool XmlParser::starts_with(const std::string& symbols) const
{
  return input != nullptr && input->compare(pos, symbols.size(), symbols) == 0;
}

bool XmlParser::consume(const char& expected)
{
  if (current() != expected) {
    return false;
  }

  ++pos;

  return true;
}

XmlParserPtr XmlParser::create() { return std::make_shared<XmlParser>(); }

}  // namespace iannotator::importers::parsers
