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
#include "PlainTxtImportLibraryContext.h"
#include "PyTorchImportLibraryContext.h"
#include "Yolo4ImportLibraryContext.h"

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
  using ILibPtr = ImagesAnnotatorDataImporters011::ILibPtr;
  using LibraryContextPtr = ImagesAnnotatorDataImporters011::LibraryContextPtr;
  using LibraryContext = ImagesAnnotatorDataImporters011::LibraryContext;
  using PlainTxtImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::PlainTxtImportLibraryContextPtr;
  using Yolo4ImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::Yolo4ImportLibraryContextPtr;
  using PyTorchImportLibraryContextPtr =
      ImagesAnnotatorDataImporters011::PyTorchImportLibraryContextPtr;
  using IImporterPtr = ImagesAnnotatorDataImporters011::IImporterPtr;
  using IImageSizeFacilityPtr =
      ImagesAnnotatorDataImporters011::IImageSizeFacilityPtr;

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
};

using LibFactoryPtr = LibFactory::LibFactoryPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBFACTORY_CLASS_H
