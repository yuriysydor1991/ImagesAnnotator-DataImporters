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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_UTF8_HELPER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_UTF8_HELPER_CLASS_H

#include <string>

#include "src/helpers/IHelper.h"

namespace iannotator::importers::helpers
{

/**
 * @brief The helper class which writes a code point out in UTF-8.
 *
 * An annotation name is user text, and both descriptor formats of this
 * library carry a way of writing a character as its code point instead of as
 * itself - the `\uXXXX` escape of JSON and the `&#NN;` reference of XML. The
 * strings the importers hand to the records are UTF-8, which is what such a
 * code point is turned into here.
 */
class Utf8Helper : virtual public IHelper
{
 public:
  virtual ~Utf8Helper() = default;
  Utf8Helper() = default;

  /**
   * @brief Appends the UTF-8 encoding of the given code point.
   *
   * @param codepoint The code point to write out.
   * @param value In-out: the text it is appended to.
   */
  inline static void append(const unsigned long& codepoint, std::string& value)
  {
    constexpr const unsigned long oneByteLast = 0x7FUL;
    constexpr const unsigned long twoByteLast = 0x7FFUL;
    constexpr const unsigned long threeByteLast = 0xFFFFUL;
    constexpr const unsigned long tailMask = 0x3FUL;
    constexpr const unsigned long tailMark = 0x80UL;

    if (codepoint <= oneByteLast) {
      append_byte(codepoint, value);
      return;
    }

    if (codepoint <= twoByteLast) {
      append_byte(0xC0UL | (codepoint >> 6U), value);
      append_byte(tailMark | (codepoint & tailMask), value);
      return;
    }

    if (codepoint <= threeByteLast) {
      append_byte(0xE0UL | (codepoint >> 12U), value);
      append_byte(tailMark | ((codepoint >> 6U) & tailMask), value);
      append_byte(tailMark | (codepoint & tailMask), value);
      return;
    }

    append_byte(0xF0UL | (codepoint >> 18U), value);
    append_byte(tailMark | ((codepoint >> 12U) & tailMask), value);
    append_byte(tailMark | ((codepoint >> 6U) & tailMask), value);
    append_byte(tailMark | (codepoint & tailMask), value);
  }

 private:
  inline static void append_byte(const unsigned long& part, std::string& value)
  {
    value += static_cast<char>(static_cast<unsigned char>(part));
  }
};

}  // namespace iannotator::importers::helpers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_UTF8_HELPER_CLASS_H
