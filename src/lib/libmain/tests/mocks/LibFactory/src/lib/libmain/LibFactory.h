#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include "IADataImportersContext.h"
#include "IADataImportersLib.h"
#include "IImageSizeFacility.h"
#include "IImporter.h"
#include "contexts/CocoImportContext.h"
#include "contexts/CreateMLImportContext.h"
#include "contexts/PascalVocImportContext.h"
#include "contexts/PlainTxtImportContext.h"
#include "contexts/PyTorchImportContext.h"
#include "contexts/UltralyticsDetectImportContext.h"
#include "contexts/UltralyticsObbImportContext.h"
#include "contexts/UltralyticsSegmentImportContext.h"
#include "contexts/Yolo4ImportContext.h"

namespace iadi0impl
{

/**
 * @brief Pure abstract stand-in for the real LibFactory, so that every
 * factory method can be mocked with an (override) MOCK_METHOD.
 */
class LibFactorySynthParent
{
 public:
  using IADataImportersLibPtr =
      ImagesAnnotatorDataImporters014::IADataImportersLibPtr;
  using IADataImportersContextPtr =
      ImagesAnnotatorDataImporters014::IADataImportersContextPtr;
  using IADataImportersContext =
      ImagesAnnotatorDataImporters014::IADataImportersContext;
  using PlainTxtImportContextPtr =
      ImagesAnnotatorDataImporters014::PlainTxtImportContextPtr;
  using Yolo4ImportContextPtr =
      ImagesAnnotatorDataImporters014::Yolo4ImportContextPtr;
  using PyTorchImportContextPtr =
      ImagesAnnotatorDataImporters014::PyTorchImportContextPtr;
  using UltralyticsDetectImportContextPtr =
      ImagesAnnotatorDataImporters014::UltralyticsDetectImportContextPtr;
  using UltralyticsObbImportContextPtr =
      ImagesAnnotatorDataImporters014::UltralyticsObbImportContextPtr;
  using UltralyticsSegmentImportContextPtr =
      ImagesAnnotatorDataImporters014::UltralyticsSegmentImportContextPtr;
  using CocoImportContextPtr =
      ImagesAnnotatorDataImporters014::CocoImportContextPtr;
  using PascalVocImportContextPtr =
      ImagesAnnotatorDataImporters014::PascalVocImportContextPtr;
  using CreateMLImportContextPtr =
      ImagesAnnotatorDataImporters014::CreateMLImportContextPtr;
  using IImporterPtr = ImagesAnnotatorDataImporters014::IImporterPtr;
  using IImageSizeFacilityPtr =
      ImagesAnnotatorDataImporters014::IImageSizeFacilityPtr;

  virtual ~LibFactorySynthParent() = default;
  LibFactorySynthParent() = default;

  virtual IADataImportersLibPtr create_default_lib() = 0;
  virtual PlainTxtImportContextPtr create_plain_txt_library_context() = 0;
  virtual Yolo4ImportContextPtr create_yolo4_library_context() = 0;
  virtual UltralyticsDetectImportContextPtr
  create_ultralytics_detect_library_context() = 0;
  virtual UltralyticsObbImportContextPtr
  create_ultralytics_obb_library_context() = 0;
  virtual UltralyticsSegmentImportContextPtr
  create_ultralytics_segment_library_context() = 0;
  virtual CocoImportContextPtr create_coco_library_context() = 0;
  virtual PascalVocImportContextPtr create_pascal_voc_library_context() = 0;
  virtual CreateMLImportContextPtr create_createml_library_context() = 0;
  virtual PyTorchImportContextPtr create_pytorch_library_context() = 0;
  virtual IADataImportersLibPtr create_appropriate_lib(
      IADataImportersContextPtr ctx) = 0;
  virtual IImporterPtr create_importer(
      const IADataImportersContextPtr& ctx) = 0;
  virtual IImageSizeFacilityPtr create_image_sizer() = 0;
};

class LibFactory : public LibFactorySynthParent
{
 public:
  using LibFactoryPtr = std::shared_ptr<LibFactory>;

  virtual ~LibFactory() = default;
  LibFactory()
  {
    if (onMockCreate) {
      onMockCreate(*this);
    }
  }

  inline static std::function<void(LibFactory& instance)> onMockCreate;

  MOCK_METHOD(IADataImportersLibPtr, create_default_lib, (), (override));
  MOCK_METHOD(PlainTxtImportContextPtr, create_plain_txt_library_context, (),
              (override));
  MOCK_METHOD(Yolo4ImportContextPtr, create_yolo4_library_context, (),
              (override));
  MOCK_METHOD(UltralyticsDetectImportContextPtr,
              create_ultralytics_detect_library_context, (), (override));
  MOCK_METHOD(UltralyticsObbImportContextPtr,
              create_ultralytics_obb_library_context, (), (override));
  MOCK_METHOD(UltralyticsSegmentImportContextPtr,
              create_ultralytics_segment_library_context, (), (override));
  MOCK_METHOD(CocoImportContextPtr, create_coco_library_context, (),
              (override));
  MOCK_METHOD(PascalVocImportContextPtr, create_pascal_voc_library_context, (),
              (override));
  MOCK_METHOD(CreateMLImportContextPtr, create_createml_library_context, (),
              (override));
  MOCK_METHOD(PyTorchImportContextPtr, create_pytorch_library_context, (),
              (override));
  MOCK_METHOD(IADataImportersLibPtr, create_appropriate_lib,
              (IADataImportersContextPtr ctx), (override));
  MOCK_METHOD(IImporterPtr, create_importer,
              (const IADataImportersContextPtr& ctx), (override));
  MOCK_METHOD(IImageSizeFacilityPtr, create_image_sizer, (), (override));

  inline static LibFactoryPtr create_factory()
  {
    return std::make_shared<LibFactory>();
  }
};

using LibFactoryPtr = LibFactory::LibFactoryPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
