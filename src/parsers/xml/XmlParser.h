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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLPARSER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLPARSER_CLASS_H

#include <cstddef>
#include <memory>
#include <string>

#include "src/parsers/xml/IXmlParser.h"
#include "src/parsers/xml/XmlNode.h"

namespace iannotator::importers::parsers
{

/**
 * @brief The recursive descent XML reader of the library.
 *
 * Reads as much of XML 1.0 as a per image dataset descriptor is ever written
 * in: the element tree, its character data with the predefined and the numeric
 * entity references resolved, the CDATA sections, and the prolog, the
 * comments, the processing instructions and the document type declaration
 * which are read over. The attributes are read over as well - see XmlNode -
 * and an internal DTD subset defining entities of its own is not read at all,
 * so a reference to one of those stays in the text as it stands.
 *
 * The instance carries the cursor of the running parse, so one instance reads
 * one document at a time. Its whole state is dropped when parse returns.
 */
class XmlParser : virtual public IXmlParser
{
 public:
  using XmlParserPtr = std::shared_ptr<XmlParser>;

  virtual ~XmlParser() = default;
  XmlParser() = default;

  virtual XmlNodePtr parse(const std::string& document) override;

  static XmlParserPtr create();

 private:
  /// @brief How deep the elements of a document may nest. The descriptors of
  /// this layout nest three levels, while a recursive descent over a file made
  /// of nothing but opening tags would otherwise walk the stack away.
  inline static constexpr const std::size_t maxDepth = 64U;

  /// @brief The longest entity reference resolved here, so that a stray
  /// ampersand of a text is told from one which opens a reference
  inline static constexpr const std::size_t maxReferenceLength = 16U;

  /**
   * @brief Reads the element under the cursor together with everything it
   * nests.
   *
   * @return Returns the read element, or a nullptr for a malformed one.
   */
  XmlNodePtr parse_element();

  /**
   * @brief Reads everything between the start and the end tag of the given
   * element into it, the cursor left standing on that end tag.
   *
   * @param element In-out: the element the read content is added to.
   *
   * @return Returns true when the content was read up to an end tag.
   */
  bool read_content(XmlNode& element);

  /**
   * @brief Reads the element or attribute name under the cursor.
   *
   * @param name Out: the read name, left alone when there is none.
   *
   * @return Returns true when a name was there.
   */
  bool read_name(std::string& name);

  /**
   * @brief Reads over the attributes of the start tag under the cursor,
   * together with the closing angle bracket of that tag.
   *
   * @param selfClosing Out: whether the tag closed the element by itself.
   *
   * @return Returns true when the whole tag was read over.
   */
  bool read_attributes(bool& selfClosing);

  /**
   * @brief Resolves the entity reference under the cursor onto the end of the
   * text being read.
   *
   * An unknown reference is no failure of the document: it is logged and left
   * in the text as it stands, since the entity it names may well be defined in
   * a document type declaration this reader does not read.
   *
   * @param value In-out: the text the resolved reference is appended to.
   */
  void read_reference(std::string& value);

  /**
   * @brief Resolves the numeric character reference of the given body.
   *
   * @param body The reference between its ampersand and its semicolon, the
   * leading hash included.
   * @param value In-out: the text the resolved reference is appended to.
   *
   * @return Returns true when the body named a code point.
   */
  static bool read_numeric_reference(const std::string& body,
                                     std::string& value);

  /**
   * @brief Reads the CDATA section under the cursor as the text it holds.
   *
   * @param value In-out: the text the section content is appended to.
   *
   * @return Returns true when the section was closed.
   */
  bool read_cdata(std::string& value);

  /**
   * @brief Reads over everything which may stand around the root element: the
   * whitespace, the prolog, the comments, the processing instructions and the
   * document type declaration.
   *
   * @return Returns true when every construct met was a closed one.
   */
  bool skip_prologue();

  /**
   * @brief Reads over the construct under the cursor.
   *
   * @param opening The symbols the construct starts with, already matched by
   * the caller.
   * @param closing The symbols it ends with.
   *
   * @return Returns true when that end was found.
   */
  bool skip_span(const std::string& opening, const std::string& closing);

  /**
   * @brief Reads over the document type declaration under the cursor, the
   * internal subset it may carry included.
   *
   * @return Returns true when the declaration was closed.
   */
  bool skip_doctype();

  void skip_blanks();
  bool at_end() const;
  /// @brief The symbol under the cursor, a NUL byte behind the end of input
  char current() const;
  /// @brief Whether the input under the cursor starts with the given symbols
  bool starts_with(const std::string& symbols) const;
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

using XmlParserPtr = XmlParser::XmlParserPtr;

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLPARSER_CLASS_H
