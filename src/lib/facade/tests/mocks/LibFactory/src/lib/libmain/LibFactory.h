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
#include "contexts/Dxxwjz2ImportContext.h"
#include "contexts/Dxxwjz2pImportContext.h"
#include "contexts/Dxxwjz3ImportContext.h"
#include "contexts/Dxxwjz3pImportContext.h"
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
      ImagesAnnotatorDataImporters015::IADataImportersLibPtr;
  using IADataImportersContextPtr =
      ImagesAnnotatorDataImporters015::IADataImportersContextPtr;
  using IADataImportersContext =
      ImagesAnnotatorDataImporters015::IADataImportersContext;
  using PlainTxtImportContextPtr =
      ImagesAnnotatorDataImporters015::PlainTxtImportContextPtr;
  using Yolo4ImportContextPtr =
      ImagesAnnotatorDataImporters015::Yolo4ImportContextPtr;
  using PyTorchImportContextPtr =
      ImagesAnnotatorDataImporters015::PyTorchImportContextPtr;
  using UltralyticsDetectImportContextPtr =
      ImagesAnnotatorDataImporters015::UltralyticsDetectImportContextPtr;
  using UltralyticsObbImportContextPtr =
      ImagesAnnotatorDataImporters015::UltralyticsObbImportContextPtr;
  using UltralyticsSegmentImportContextPtr =
      ImagesAnnotatorDataImporters015::UltralyticsSegmentImportContextPtr;
  using CocoImportContextPtr =
      ImagesAnnotatorDataImporters015::CocoImportContextPtr;
  using PascalVocImportContextPtr =
      ImagesAnnotatorDataImporters015::PascalVocImportContextPtr;
  using CreateMLImportContextPtr =
      ImagesAnnotatorDataImporters015::CreateMLImportContextPtr;
  using Dxxwjz2pImportContextPtr =
      ImagesAnnotatorDataImporters015::Dxxwjz2pImportContextPtr;
  using Dxxwjz3pImportContextPtr =
      ImagesAnnotatorDataImporters015::Dxxwjz3pImportContextPtr;
  using Dxxwjz2ImportContextPtr =
      ImagesAnnotatorDataImporters015::Dxxwjz2ImportContextPtr;
  using Dxxwjz3ImportContextPtr =
      ImagesAnnotatorDataImporters015::Dxxwjz3ImportContextPtr;
  using IImporterPtr = ImagesAnnotatorDataImporters015::IImporterPtr;
  using IImageSizeFacilityPtr =
      ImagesAnnotatorDataImporters015::IImageSizeFacilityPtr;

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
  virtual Dxxwjz2pImportContextPtr create_dxxwjz2p_library_context() = 0;
  virtual Dxxwjz3pImportContextPtr create_dxxwjz3p_library_context() = 0;
  virtual Dxxwjz2ImportContextPtr create_dxxwjz2_library_context() = 0;
  virtual Dxxwjz3ImportContextPtr create_dxxwjz3_library_context() = 0;
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
  MOCK_METHOD(Dxxwjz2pImportContextPtr, create_dxxwjz2p_library_context, (),
              (override));
  MOCK_METHOD(Dxxwjz3pImportContextPtr, create_dxxwjz3p_library_context, (),
              (override));
  MOCK_METHOD(Dxxwjz2ImportContextPtr, create_dxxwjz2_library_context, (),
              (override));
  MOCK_METHOD(Dxxwjz3ImportContextPtr, create_dxxwjz3_library_context, (),
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
