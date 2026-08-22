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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H

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
 * The library main factory to operate with the implementation.
 * Create appropriate instances in accordance of provided data
 * by the given context or by simply calling appropriate method.
 */
class LibFactory
{
 public:
  using LibFactoryPtr = std::shared_ptr<LibFactory>;
  using ILibPtr = ImagesAnnotatorDataImporters013::ILibPtr;
  using LibraryContextPtr = ImagesAnnotatorDataImporters013::LibraryContextPtr;
  using LibraryContext = ImagesAnnotatorDataImporters013::LibraryContext;
  using PlainTxtImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::PlainTxtImportLibraryContextPtr;
  using Yolo4ImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::Yolo4ImportLibraryContextPtr;
  using PyTorchImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::PyTorchImportLibraryContextPtr;
  using UltralyticsDetectImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::UltralyticsDetectImportLibraryContextPtr;
  using UltralyticsObbImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::UltralyticsObbImportLibraryContextPtr;
  using UltralyticsSegmentImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::
          UltralyticsSegmentImportLibraryContextPtr;
  using CocoImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::CocoImportLibraryContextPtr;
  using PascalVocImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::PascalVocImportLibraryContextPtr;
  using CreateMLImportLibraryContextPtr =
      ImagesAnnotatorDataImporters013::CreateMLImportLibraryContextPtr;
  using IImporterPtr = ImagesAnnotatorDataImporters013::IImporterPtr;
  using IImageSizeFacilityPtr =
      ImagesAnnotatorDataImporters013::IImageSizeFacilityPtr;

  virtual ~LibFactory() = default;
  LibFactory() = default;

  /**
   * @brief Creates the default library implementation. May be overriden
   * in descendants.
   *
   * @return Returns the default library implementation. Currently returns
   * the LibMain class instance.
   */
  virtual ILibPtr create_default_lib();

  /**
   * @brief Creates an empty context of the plain text dataset layout.
   *
   * @return Returns an empty PlainTxtImportLibraryContext instance.
   */
  virtual PlainTxtImportLibraryContextPtr create_plain_txt_library_context();

  /**
   * @brief Creates an empty context of the YOLO v4 dataset layout.
   *
   * @return Returns an empty Yolo4ImportLibraryContext instance.
   */
  virtual Yolo4ImportLibraryContextPtr create_yolo4_library_context();

  /**
   * @brief Creates an empty context of the Ultralytics YOLO detection dataset
   * layout.
   *
   * @return Returns an empty UltralyticsDetectImportLibraryContext instance.
   */
  virtual UltralyticsDetectImportLibraryContextPtr
  create_ultralytics_detect_library_context();

  /**
   * @brief Creates an empty context of the Ultralytics YOLO oriented bounding
   * box dataset layout.
   *
   * @return Returns an empty UltralyticsObbImportLibraryContext instance.
   */
  virtual UltralyticsObbImportLibraryContextPtr
  create_ultralytics_obb_library_context();

  /**
   * @brief Creates an empty context of the Ultralytics YOLO instance
   * segmentation dataset layout.
   *
   * @return Returns an empty UltralyticsSegmentImportLibraryContext instance.
   */
  virtual UltralyticsSegmentImportLibraryContextPtr
  create_ultralytics_segment_library_context();

  /**
   * @brief Creates an empty context of the COCO object detection dataset
   * layout.
   *
   * @return Returns an empty CocoImportLibraryContext instance.
   */
  virtual CocoImportLibraryContextPtr create_coco_library_context();

  /**
   * @brief Creates an empty context of the Pascal VOC dataset layout.
   *
   * @return Returns an empty PascalVocImportLibraryContext instance.
   */
  virtual PascalVocImportLibraryContextPtr create_pascal_voc_library_context();

  /**
   * @brief Creates an empty context of the Create ML object detection dataset
   * layout.
   *
   * @return Returns an empty CreateMLImportLibraryContext instance.
   */
  virtual CreateMLImportLibraryContextPtr create_createml_library_context();

  /**
   * @brief Creates an empty context of the PyTorch Vision dataset layout.
   *
   * @return Returns an empty PyTorchImportLibraryContext instance.
   */
  virtual PyTorchImportLibraryContextPtr create_pytorch_library_context();

  /**
   * @brief Creates appropriate library implementation instance
   * with accordance of provided data in the given context.
   *
   * @return Returns the appropriate library implementation if any
   * or a nullptr in case of any error. Currently returns the LibMain class
   * instance only.
   */
  virtual ILibPtr create_appropriate_lib(LibraryContextPtr ctx);

  /**
   * @brief Creates the importer implementing the dataset layout of the given
   * context.
   *
   * @param ctx The LibraryContext descendant naming the wanted layout.
   *
   * @return Returns a new importer or a nullptr for an empty context.
   */
  virtual IImporterPtr create_importer(const LibraryContextPtr& ctx);

  /**
   * @brief Creates the image size reader the library ships itself.
   *
   * @return Returns a new reader when this build found OpenCV, and a nullptr
   * when it did not. A nullptr only means the consumer has to supply one of
   * its own through LibraryContext::set_image_sizer().
   */
  virtual IImageSizeFacilityPtr create_image_sizer();

  static LibFactoryPtr create_factory();

 private:
  /**
   * @brief Creates the importer of one family of the dataset layouts.
   *
   * The wanted layout is the very type of the context descendant, so the
   * lookup is a cast per layout - and the layouts are grouped into the three
   * families they are written by, so that no single method carries the whole
   * list of them.
   *
   * @param ctx The LibraryContext descendant naming the wanted layout.
   *
   * @return Returns a new importer, or a nullptr when the context names no
   * layout of that family.
   */
  static IImporterPtr create_plain_importer(const LibraryContextPtr& ctx);
  /// @copydoc LibFactory::create_plain_importer
  static IImporterPtr create_ultralytics_importer(const LibraryContextPtr& ctx);
  /// @copydoc LibFactory::create_plain_importer
  static IImporterPtr create_descriptor_importer(const LibraryContextPtr& ctx);
};

using LibFactoryPtr = LibFactory::LibFactoryPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
