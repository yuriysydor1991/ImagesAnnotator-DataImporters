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

#include "IADataImportersFacade.h"

#include <cassert>
#include <memory>
#include <string>

#include "project-global-decls.h"
#include "src/lib/libmain/LibFactory.h"
#include "src/log/log.h"

namespace ImagesAnnotatorDataImporters014
{

PlainTxtImportContextPtr
IADataImportersFacade::create_plain_txt_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_plain_txt_library_context();
}

Yolo4ImportContextPtr IADataImportersFacade::create_yolo4_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_yolo4_library_context();
}

UltralyticsDetectImportContextPtr
IADataImportersFacade::create_ultralytics_detect_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_detect_library_context();
}

UltralyticsObbImportContextPtr
IADataImportersFacade::create_ultralytics_obb_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_obb_library_context();
}

UltralyticsSegmentImportContextPtr
IADataImportersFacade::create_ultralytics_segment_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_ultralytics_segment_library_context();
}

CocoImportContextPtr IADataImportersFacade::create_coco_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_coco_library_context();
}

PascalVocImportContextPtr
IADataImportersFacade::create_pascal_voc_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_pascal_voc_library_context();
}

CreateMLImportContextPtr
IADataImportersFacade::create_createml_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_createml_library_context();
}

PyTorchImportContextPtr IADataImportersFacade::create_pytorch_library_context()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_pytorch_library_context();
}

IADataImportersLibPtr IADataImportersFacade::create_default_lib()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_default_lib();
}

IADataImportersLibPtr IADataImportersFacade::create_library(
    IADataImportersContextPtr ctx)
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_appropriate_lib(ctx);
}

IImporterPtr IADataImportersFacade::create_importer(
    const IADataImportersContextPtr& ctx)
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_importer(ctx);
}

IImageSizeFacilityPtr IADataImportersFacade::create_image_sizer()
{
  auto libFactory = iadi0impl::LibFactory::create_factory();

  assert(libFactory != nullptr);

  return libFactory->create_image_sizer();
}

std::string IADataImportersFacade::library_version()
{
  return project_decls::PROJECT_BUILD_VERSION;
}

void IADataImportersFacade::accept_real_logger(
    const std::shared_ptr<logger::ILogger>& realLogger)
{
  LOG_INIT_REAL_LOGGER(realLogger);
}

}  // namespace ImagesAnnotatorDataImporters014
