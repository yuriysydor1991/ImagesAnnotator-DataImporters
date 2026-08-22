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

#include "LibraryFacade.h"

#include <cassert>
#include <memory>
#include <string>

#include "project-global-decls.h"
#include "src/lib/libmain/LibFactory.h"
#include "src/log/log.h"

namespace ImagesAnnotatorDataImporters013
{

PlainTxtImportLibraryContextPtr
LibraryFacade::create_plain_txt_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_plain_txt_library_context();
}

Yolo4ImportLibraryContextPtr LibraryFacade::create_yolo4_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_yolo4_library_context();
}

UltralyticsDetectImportLibraryContextPtr
LibraryFacade::create_ultralytics_detect_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_detect_library_context();
}

UltralyticsObbImportLibraryContextPtr
LibraryFacade::create_ultralytics_obb_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_obb_library_context();
}

UltralyticsSegmentImportLibraryContextPtr
LibraryFacade::create_ultralytics_segment_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_segment_library_context();
}

CocoImportLibraryContextPtr LibraryFacade::create_coco_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_coco_library_context();
}

PascalVocImportLibraryContextPtr
LibraryFacade::create_pascal_voc_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_pascal_voc_library_context();
}

CreateMLImportLibraryContextPtr LibraryFacade::create_createml_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_createml_library_context();
}

PyTorchImportLibraryContextPtr LibraryFacade::create_pytorch_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_pytorch_library_context();
}

ILibPtr LibraryFacade::create_default_lib()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_default_lib();
}

ILibPtr LibraryFacade::create_library(LibraryContextPtr ctx)
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_appropriate_lib(ctx);
}

IImporterPtr LibraryFacade::create_importer(const LibraryContextPtr& ctx)
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_importer(ctx);
}

IImageSizeFacilityPtr LibraryFacade::create_image_sizer()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_image_sizer();
}

std::string LibraryFacade::library_version()
{
  return project_decls::PROJECT_BUILD_VERSION;
}

void LibraryFacade::accept_real_logger(
    const std::shared_ptr<logger::ILogger>& realLogger)
{
  LOG_INIT_REAL_LOGGER(realLogger);
}

}  // namespace ImagesAnnotatorDataImporters013
