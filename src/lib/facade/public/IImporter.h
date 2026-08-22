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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMPORTER_ABSTRACT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMPORTER_ABSTRACT_CLASS_H

#include <memory>

#include "ImportersAPI.h"

namespace ImagesAnnotatorDataImporters013
{

/**
 * @brief Only declared here, since the LibraryContext holds an IImporterPtr
 * of its own and its header is the one which includes this file.
 */
class IADI_API LibraryContext;
using LibraryContextPtr = std::shared_ptr<LibraryContext>;

/**
 * @brief The abstract class to define the interface for the all available
 * annotation data importers.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API IImporter
{
 public:
  using IImporterPtr = std::shared_ptr<IImporter>;

  virtual ~IImporter() = default;
  IImporter() = default;

  /**
   * @brief Reads the dataset named by the context in the layout this importer
   * implements and merges the records it recovers into the database of that
   * very same context, skipping the entries it cannot process.
   *
   * @param ictx The filled library context. Both its import path and its
   * database are mandatory. The layout the context type names is not looked at
   * here: the layout read is the one of this very importer. An implementation
   * must not keep the context beyond the call, since a context holding this
   * importer back would close a pointer cycle.
   *
   * @return Returns true when the import as a whole ran through.
   */
  virtual bool import_db(LibraryContextPtr ictx) = 0;
};

using IImporterPtr = IImporter::IImporterPtr;

}  // namespace ImagesAnnotatorDataImporters013

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMPORTER_ABSTRACT_CLASS_H
