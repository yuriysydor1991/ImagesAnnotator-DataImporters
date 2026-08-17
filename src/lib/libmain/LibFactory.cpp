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

#include "src/lib/libmain/LibFactory.h"

#include <cassert>
#include <memory>

#include "PlainTxtImportLibraryContext.h"
#include "PyTorchImportLibraryContext.h"
#include "Yolo4ImportLibraryContext.h"
#include "src/importers/PlainTxt/PlainTxtFolder2DBImporter.h"
#include "src/importers/PyTorch/PyTorchVisionFolder2DBImporter.h"
#include "src/importers/Yolo4/Yolo4Folder2DBImporter.h"
#include "src/lib/libmain/LibMain.h"
#include "src/log/log.h"
#include "src/sizers/ImageSizeFactory.h"

namespace iadi0impl
{

using namespace ImagesAnnotatorDataImporters011;

LibFactory::ILibPtr LibFactory::create_default_lib()
{
  return LibMain::create();
}

LibFactory::PlainTxtImportLibraryContextPtr
LibFactory::create_plain_txt_library_context()
{
  return std::make_shared<PlainTxtImportLibraryContext>();
}

LibFactory::Yolo4ImportLibraryContextPtr
LibFactory::create_yolo4_library_context()
{
  return std::make_shared<Yolo4ImportLibraryContext>();
}

LibFactory::PyTorchImportLibraryContextPtr
LibFactory::create_pytorch_library_context()
{
  return std::make_shared<PyTorchImportLibraryContext>();
}

LibFactory::ILibPtr LibFactory::create_appropriate_lib(
    [[maybe_unused]] LibraryContextPtr ctx)
{
  assert(ctx != nullptr);

  return create_default_lib();
}

// The wanted layout is the very type of the context descendant. A cast per
// layout keeps that knowledge here, where the concrete importer classes are
// already known, instead of in the installed headers.
LibFactory::IImporterPtr LibFactory::create_importer(
    const LibraryContextPtr& ctx)
{
  if (std::dynamic_pointer_cast<PlainTxtImportLibraryContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::PlainTxtFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<Yolo4ImportLibraryContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::Yolo4Folder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<PyTorchImportLibraryContext>(ctx) != nullptr) {
    return std::make_shared<
        iannotator::importers::PyTorchVisionFolder2DBImporter>();
  }

  LOGE("No library context of a known dataset layout given");

  return nullptr;
}

LibFactory::IImageSizeFacilityPtr LibFactory::create_image_sizer()
{
  return iannotator::importers::sizers::create_builtin_image_sizer();
}

LibFactoryPtr LibFactory::create_factory()
{
  return std::make_shared<LibFactory>();
}

}  // namespace iadi0impl
