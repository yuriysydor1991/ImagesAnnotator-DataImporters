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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IXMLPARSER_ABSTRACT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IXMLPARSER_ABSTRACT_CLASS_H

#include <memory>
#include <string>

#include "src/parsers/xml/XmlNode.h"

namespace iannotator::importers::parsers
{

/**
 * @brief The interface of the XML document reader the XML descriptor layouts
 * are read through.
 *
 * The importers depend on this abstraction alone, so the reader behind it - an
 * XML library, were one ever linked, instead of the XmlParser written here -
 * is exchanged without any of them being touched.
 */
class IXmlParser
{
 public:
  using IXmlParserPtr = std::shared_ptr<IXmlParser>;

  virtual ~IXmlParser() = default;
  IXmlParser() = default;

  /**
   * @brief Reads the whole given document into its element tree.
   *
   * @param document The XML text to read.
   *
   * @return Should return the root element, or a nullptr when the text is no
   * single well formed XML document.
   */
  virtual XmlNodePtr parse(const std::string& document) = 0;
};

using IXmlParserPtr = IXmlParser::IXmlParserPtr;

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IXMLPARSER_ABSTRACT_CLASS_H
