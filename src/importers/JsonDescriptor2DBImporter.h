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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONDESCRIPTOR2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONDESCRIPTOR2DBIMPORTER_CLASS_H

#include <filesystem>
#include <vector>

#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"
#include "src/parsers/json/JsonValue.h"

namespace iannotator::importers
{

/**
 * @brief The common half of the importers whose whole dataset descriptor is a
 * single JSON document: the COCO one and the Create ML one.
 *
 * Neither layout names its descriptor in a way that could simply be opened:
 * the file the sibling exporters library writes is only the most expected of
 * the names a dataset of that layout arrives under, and a directory may hold
 * more than one JSON file. So a descendant hands over the files its layout may
 * be described by, most expected first, and this class reads them until one is
 * the descriptor - which is what its holds_dataset tells.
 */
class JsonDescriptor2DBImporter : public Folder2DBImporter
{
 public:
  virtual ~JsonDescriptor2DBImporter() = default;
  JsonDescriptor2DBImporter() = default;

 protected:
  using JsonValue = parsers::JsonValue;
  using JsonValuePtr = parsers::JsonValuePtr;
  using Paths = std::vector<std::filesystem::path>;

  /**
   * @brief Reads the first of the given files which holds the descriptor of
   * the layout the descendant implements.
   *
   * @param candidates The files to try, in the order they are to be tried in.
   * @param descriptorPath Out: the path the descriptor was read from, left
   * alone when none of the candidates held one.
   *
   * @return Returns the parsed descriptor, or a nullptr when none of the
   * candidates was one.
   */
  JsonValuePtr read_descriptor(const Paths& candidates,
                               std::filesystem::path& descriptorPath) const;

  /**
   * @brief Tells whether the given document is a descriptor of the implemented
   * layout.
   *
   * @param root The parsed JSON document to look at.
   *
   * @return Should return true when the document is the descriptor of a
   * dataset of this layout, and false for any other JSON file which happened
   * to lie beside it.
   */
  virtual bool holds_dataset(const JsonValue& root) const = 0;

  /**
   * @brief Appends the `*.json` files of the given directory to the
   * candidates.
   *
   * @param dirPath The directory to walk. A missing one appends nothing.
   * @param candidates In-out: the list the found files are appended to, sorted
   * by name so that one and the same directory imports one and the same way
   * twice, and the files it already names left out.
   */
  static void collect_descriptors(const std::filesystem::path& dirPath,
                                  Paths& candidates);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONDESCRIPTOR2DBIMPORTER_CLASS_H
