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

#include "contexts/CocoImportContext.h"
#include "contexts/CreateMLImportContext.h"
#include "contexts/PascalVocImportContext.h"
#include "contexts/PlainTxtImportContext.h"
#include "contexts/PyTorchImportContext.h"
#include "contexts/UltralyticsDetectImportContext.h"
#include "contexts/UltralyticsObbImportContext.h"
#include "contexts/UltralyticsSegmentImportContext.h"
#include "contexts/Yolo4ImportContext.h"
#include "src/importers/Coco/CocoFolder2DBImporter.h"
#include "src/importers/CreateML/CreateMLFolder2DBImporter.h"
#include "src/importers/PascalVoc/PascalVocFolder2DBImporter.h"
#include "src/importers/PlainTxt/PlainTxtFolder2DBImporter.h"
#include "src/importers/PyTorch/PyTorchVisionFolder2DBImporter.h"
#include "src/importers/Ultralytics/UltralyticsDetectFolder2DBImporter.h"
#include "src/importers/Ultralytics/UltralyticsObbFolder2DBImporter.h"
#include "src/importers/Ultralytics/UltralyticsSegmentFolder2DBImporter.h"
#include "src/importers/Yolo4/Yolo4Folder2DBImporter.h"
#include "src/lib/libmain/LibMain.h"
#include "src/log/log.h"
#include "src/sizers/ImageSizeFactory.h"

namespace iadi0impl
{

using namespace ImagesAnnotatorDataImporters014;

LibFactory::IADataImportersLibPtr LibFactory::create_default_lib()
{
  return LibMain::create();
}

LibFactory::PlainTxtImportContextPtr
LibFactory::create_plain_txt_library_context()
{
  return std::make_shared<PlainTxtImportContext>();
}

LibFactory::Yolo4ImportContextPtr LibFactory::create_yolo4_library_context()
{
  return std::make_shared<Yolo4ImportContext>();
}

LibFactory::UltralyticsDetectImportContextPtr
LibFactory::create_ultralytics_detect_library_context()
{
  return std::make_shared<UltralyticsDetectImportContext>();
}

LibFactory::UltralyticsObbImportContextPtr
LibFactory::create_ultralytics_obb_library_context()
{
  return std::make_shared<UltralyticsObbImportContext>();
}

LibFactory::UltralyticsSegmentImportContextPtr
LibFactory::create_ultralytics_segment_library_context()
{
  return std::make_shared<UltralyticsSegmentImportContext>();
}

LibFactory::CocoImportContextPtr LibFactory::create_coco_library_context()
{
  return std::make_shared<CocoImportContext>();
}

LibFactory::PascalVocImportContextPtr
LibFactory::create_pascal_voc_library_context()
{
  return std::make_shared<PascalVocImportContext>();
}

LibFactory::CreateMLImportContextPtr
LibFactory::create_createml_library_context()
{
  return std::make_shared<CreateMLImportContext>();
}

LibFactory::PyTorchImportContextPtr LibFactory::create_pytorch_library_context()
{
  return std::make_shared<PyTorchImportContext>();
}

LibFactory::IADataImportersLibPtr LibFactory::create_appropriate_lib(
    [[maybe_unused]] IADataImportersContextPtr ctx)
{
  assert(ctx != nullptr);

  return create_default_lib();
}

// The wanted layout is the very type of the context descendant. A cast per
// layout keeps that knowledge here, where the concrete importer classes are
// already known, instead of in the installed headers.
LibFactory::IImporterPtr LibFactory::create_importer(
    const IADataImportersContextPtr& ctx)
{
  auto importer = create_plain_importer(ctx);

  if (importer == nullptr) {
    importer = create_ultralytics_importer(ctx);
  }

  if (importer == nullptr) {
    importer = create_descriptor_importer(ctx);
  }

  if (importer == nullptr) {
    LOGE("No library context of a known dataset layout given");
  }

  return importer;
}

LibFactory::IImporterPtr LibFactory::create_plain_importer(
    const IADataImportersContextPtr& ctx)
{
  if (std::dynamic_pointer_cast<PlainTxtImportContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::PlainTxtFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<Yolo4ImportContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::Yolo4Folder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<PyTorchImportContext>(ctx) != nullptr) {
    return std::make_shared<
        iannotator::importers::PyTorchVisionFolder2DBImporter>();
  }

  return nullptr;
}

LibFactory::IImporterPtr LibFactory::create_ultralytics_importer(
    const IADataImportersContextPtr& ctx)
{
  if (std::dynamic_pointer_cast<UltralyticsDetectImportContext>(ctx) !=
      nullptr) {
    return std::make_shared<
        iannotator::importers::UltralyticsDetectFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<UltralyticsObbImportContext>(ctx) != nullptr) {
    return std::make_shared<
        iannotator::importers::UltralyticsObbFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<UltralyticsSegmentImportContext>(ctx) !=
      nullptr) {
    return std::make_shared<
        iannotator::importers::UltralyticsSegmentFolder2DBImporter>();
  }

  return nullptr;
}

LibFactory::IImporterPtr LibFactory::create_descriptor_importer(
    const IADataImportersContextPtr& ctx)
{
  if (std::dynamic_pointer_cast<CocoImportContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::CocoFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<PascalVocImportContext>(ctx) != nullptr) {
    return std::make_shared<
        iannotator::importers::PascalVocFolder2DBImporter>();
  }

  if (std::dynamic_pointer_cast<CreateMLImportContext>(ctx) != nullptr) {
    return std::make_shared<iannotator::importers::CreateMLFolder2DBImporter>();
  }

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
