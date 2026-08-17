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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H

#include <memory>

#include "ILib.h"
#include "LibraryContext.h"

namespace iadi0impl
{

/**
 * @brief The default library implementation class.
 * Class is designed to hold the main library implementation code, which is
 * the one shot import run: the perform_import method builds the importer of
 * the layout named by the given context and drives it over the directory that
 * very same context points at.
 */
class LibMain : public ImagesAnnotatorDataImporters011::ILib
{
 public:
  using LibMainPtr = std::shared_ptr<LibMain>;
  using LibraryContextPtr = ImagesAnnotatorDataImporters011::LibraryContextPtr;

  virtual ~LibMain() = default;
  LibMain();

  /**
   * @brief The implemented library interface method derived from an ILib
   * abstract class. Designed to contain the main library code implementation.
   *
   * Builds the importer of the context layout and runs it over that very same
   * context. The importer that was used is provided back through the context
   * importer field.
   *
   * @param ctx A filled LibraryContext descendant with appropriate data
   * to perform it's actions.
   *
   * @return Returns a true value on the success and false in case of any
   * error.
   */
  virtual bool perform_import(LibraryContextPtr ctx) override;

  static LibMainPtr create();
};

using LibMainPtr = LibMain::LibMainPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
