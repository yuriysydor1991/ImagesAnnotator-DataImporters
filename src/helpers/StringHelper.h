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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_STRING_HELPER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_STRING_HELPER_CLASS_H

#include <string>

#include "src/helpers/IHelper.h"

namespace iannotator::importers::helpers
{

/**
 * @brief The helper class for the text every dataset descriptor is written in.
 *
 * The layouts read here are written by hand as much as by a tool - the
 * darknet obj.data entries carry blanks around their equals sign, an XML
 * element may hold its value on a line of its own, a YAML entry is indented -
 * so the very same trimming is what tells the value of such an entry from the
 * whitespace it was laid out with.
 */
class StringHelper : virtual public IHelper
{
 public:
  /// @brief The whitespace a descriptor of these layouts is laid out with
  inline static const std::string blanks = " \t\r\n";

  virtual ~StringHelper() = default;
  StringHelper() = default;

  /**
   * @brief Drops the leading and the trailing whitespace of the given value.
   *
   * @param value The text to trim.
   *
   * @return Returns the trimmed text, an empty string for a value which is
   * whitespace and nothing else.
   */
  inline static std::string trim(const std::string& value)
  {
    const auto first = value.find_first_not_of(blanks);

    if (first == std::string::npos) {
      return {};
    }

    return value.substr(first, value.find_last_not_of(blanks) - first + 1U);
  }
};

}  // namespace iannotator::importers::helpers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_STRING_HELPER_CLASS_H
