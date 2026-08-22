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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONPARSER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONPARSER_CLASS_H

#include <cstddef>
#include <memory>
#include <string>

#include "src/parsers/json/IJsonParser.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers::parsers
{

/**
 * @brief The recursive descent JSON reader of the library.
 *
 * Reads the whole of RFC 8259 - the four scalar kinds, the two containers and
 * every string escape, the `\uXXXX` surrogate pairs included - and reads no
 * more than that: a trailing comma, a comment or an unquoted key is a
 * malformed document here, exactly as it is for the readers the descriptors
 * of these layouts are otherwise handed to.
 *
 * The instance carries the cursor of the running parse, so one instance reads
 * one document at a time. Its whole state is dropped when parse returns.
 */
class JsonParser : virtual public IJsonParser
{
 public:
  using JsonParserPtr = std::shared_ptr<JsonParser>;

  virtual ~JsonParser() = default;
  JsonParser() = default;

  virtual JsonValuePtr parse(const std::string& document) override;

  static JsonParserPtr create();

 private:
  /// @brief How deep the containers of a document may nest. The descriptors
  /// of these layouts nest four levels at the most, while a recursive descent
  /// over a file made of nothing but opening brackets would otherwise walk
  /// the stack away.
  inline static constexpr const std::size_t maxDepth = 64U;

  JsonValuePtr parse_value();
  JsonValuePtr parse_object();
  JsonValuePtr parse_array();
  JsonValuePtr parse_string();
  JsonValuePtr parse_number();
  JsonValuePtr parse_keyword();

  /**
   * @brief Reads the quoted string under the cursor, the escapes resolved.
   *
   * @param value Out: the read text, left alone on a failure.
   *
   * @return Returns true when a whole string was there.
   */
  bool read_string(std::string& value);
  /**
   * @brief Resolves the one escape sequence behind the backslash under the
   * cursor onto the end of the string being read.
   *
   * @param value In-out: the text the resolved escape is appended to.
   *
   * @return Returns true when the sequence was one of the format.
   */
  bool read_escape(std::string& value);
  /**
   * @brief Reads the four hexadecimal digits of a `\uXXXX` escape.
   *
   * @param code Out: the read code unit, left alone on a failure.
   *
   * @return Returns true when four hexadecimal digits were there.
   */
  bool read_code_unit(unsigned& code);

  void skip_blanks();
  bool at_end() const;
  /// @brief The symbol under the cursor, a NUL byte behind the end of input
  char current() const;
  /**
   * @brief Steps the cursor over the expected symbol.
   *
   * @param expected The symbol the cursor is to stand on.
   *
   * @return Returns true when it did and the cursor moved on.
   */
  bool consume(const char& expected);

  /// @brief The document of the running parse. Held for the call only, so
  /// nothing of the caller string is copied into this instance.
  const std::string* input{nullptr};
  std::size_t pos{0U};
  std::size_t depth{0U};
};

using JsonParserPtr = JsonParser::JsonParserPtr;

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONPARSER_CLASS_H
