#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include "IImageSizeFacility.h"
#include "IImporter.h"
#include "ILib.h"
#include "LibraryContext.h"
#include "contexts/CocoImportLibraryContext.h"
#include "contexts/CreateMLImportLibraryContext.h"
#include "contexts/PascalVocImportLibraryContext.h"
#include "contexts/PlainTxtImportLibraryContext.h"
#include "contexts/PyTorchImportLibraryContext.h"
#include "contexts/UltralyticsDetectImportLibraryContext.h"
#include "contexts/UltralyticsObbImportLibraryContext.h"
#include "contexts/UltralyticsSegmentImportLibraryContext.h"
#include "contexts/Yolo4ImportLibraryContext.h"

namespace iadi0impl
{

/**
 * @brief Pure abstract stand-in for the real LibFactory, so that every
 * factory method can be mocked with an (override) MOCK_METHOD.
 */
class LibFactorySynthParent
{
 public:
  using ILibPtr = ImagesAnnotatorDataImporters011::ILibPtr;
  using LibraryContextPtr = ImagesAnnotatorDataImporters011::LibraryContextPtr;
  using LibraryContext = ImagesAnnotatorDataImporters011::LibraryContext;
  using PlainTxtImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::PlainTxtImportLibraryContextPtr;
  using Yolo4ImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::Yolo4ImportLibraryContextPtr;
  using PyTorchImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::PyTorchImportLibraryContextPtr;
  using UltralyticsDetectImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::UltralyticsDetectImportLibraryContextPtr;
  using UltralyticsObbImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::UltralyticsObbImportLibraryContextPtr;
  using UltralyticsSegmentImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::
          UltralyticsSegmentImportLibraryContextPtr;
  using CocoImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::CocoImportLibraryContextPtr;
  using PascalVocImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::PascalVocImportLibraryContextPtr;
  using CreateMLImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::CreateMLImportLibraryContextPtr;
  using IImporterPtr = ImagesAnnotatorDataImporters011::IImporterPtr;
  using IImageSizeFacilityPtr =
      ImagesAnnotatorDataImporters011::IImageSizeFacilityPtr;

  virtual ~LibFactorySynthParent() = default;
  LibFactorySynthParent() = default;

  virtual ILibPtr create_default_lib() = 0;
  virtual PlainTxtImportLibraryContextPtr
  create_plain_txt_library_context() = 0;
  virtual Yolo4ImportLibraryContextPtr create_yolo4_library_context() = 0;
  virtual UltralyticsDetectImportLibraryContextPtr
  create_ultralytics_detect_library_context() = 0;
  virtual UltralyticsObbImportLibraryContextPtr
  create_ultralytics_obb_library_context() = 0;
  virtual UltralyticsSegmentImportLibraryContextPtr
  create_ultralytics_segment_library_context() = 0;
  virtual CocoImportLibraryContextPtr create_coco_library_context() = 0;
  virtual PascalVocImportLibraryContextPtr
  create_pascal_voc_library_context() = 0;
  virtual CreateMLImportLibraryContextPtr create_createml_library_context() = 0;
  virtual PyTorchImportLibraryContextPtr create_pytorch_library_context() = 0;
  virtual ILibPtr create_appropriate_lib(LibraryContextPtr ctx) = 0;
  virtual IImporterPtr create_importer(const LibraryContextPtr& ctx) = 0;
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

  MOCK_METHOD(ILibPtr, create_default_lib, (), (override));
  MOCK_METHOD(PlainTxtImportLibraryContextPtr, create_plain_txt_library_context,
              (), (override));
  MOCK_METHOD(Yolo4ImportLibraryContextPtr, create_yolo4_library_context, (),
              (override));
  MOCK_METHOD(UltralyticsDetectImportLibraryContextPtr,
              create_ultralytics_detect_library_context, (), (override));
  MOCK_METHOD(UltralyticsObbImportLibraryContextPtr,
              create_ultralytics_obb_library_context, (), (override));
  MOCK_METHOD(UltralyticsSegmentImportLibraryContextPtr,
              create_ultralytics_segment_library_context, (), (override));
  MOCK_METHOD(CocoImportLibraryContextPtr, create_coco_library_context, (),
              (override));
  MOCK_METHOD(PascalVocImportLibraryContextPtr,
              create_pascal_voc_library_context, (), (override));
  MOCK_METHOD(CreateMLImportLibraryContextPtr, create_createml_library_context,
              (), (override));
  MOCK_METHOD(PyTorchImportLibraryContextPtr, create_pytorch_library_context,
              (), (override));
  MOCK_METHOD(ILibPtr, create_appropriate_lib, (LibraryContextPtr ctx),
              (override));
  MOCK_METHOD(IImporterPtr, create_importer, (const LibraryContextPtr& ctx),
              (override));
  MOCK_METHOD(IImageSizeFacilityPtr, create_image_sizer, (), (override));

  inline static LibFactoryPtr create_factory()
  {
    return std::make_shared<LibFactory>();
  }
};

using LibFactoryPtr = LibFactory::LibFactoryPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
