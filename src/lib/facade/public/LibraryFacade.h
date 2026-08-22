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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYFACADE_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYFACADE_CLASS_H

#include <memory>
#include <string>

#include "IImageSizeFacility.h"
#include "IImporter.h"
#include "ILib.h"
#include "ImportersAPI.h"
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

/**
 * @brief The logging subsystem interface every project built from the
 * cpp-app-template shares. The definition compiled into this library is token
 * identical to the one of the project which uses it, so this installed header
 * only forward declares it and never collides with the consumer own
 * logger::ILogger definition.
 */
namespace logger
{
class ILogger;
}  // namespace logger

namespace ImagesAnnotatorDataImporters013
{

/**
 * @brief The facade interface class of the library.
 *
 * This is the only entry point a consuming project needs: every concrete
 * implementation class of the library stays hidden behind the abstract
 * interfaces returned from here.
 *
 * @code
 * namespace iadd = ImagesAnnotatorDataDrivers011;
 * namespace iadi = ImagesAnnotatorDataImporters013;
 *
 * auto db = iadd::LibraryFacade::create_annotations_db();
 *
 * auto ctx = iadi::LibraryFacade::create_yolo4_library_context();
 * ctx->set_import_path("/tmp/yolo-dataset");
 * ctx->set_db(db);
 *
 * auto importer = iadi::LibraryFacade::create_importer(ctx);
 *
 * return importer != nullptr && importer->import_db(ctx) &&
 *        db->store_db("project.json");
 * @endcode
 *
 * Current file is a target for the library header installation.
 */
class IADI_API LibraryFacade
{
 public:
  virtual ~LibraryFacade() = default;
  LibraryFacade() = default;

  /**
   * @brief Factory method to create an empty library context of the plain
   * text dataset layout.
   *
   * @return Returns a new PlainTxtImportLibraryContext class instance.
   */
  static PlainTxtImportLibraryContextPtr create_plain_txt_library_context();

  /**
   * @brief Factory method to create an empty library context of the YOLO v4
   * dataset layout.
   *
   * @return Returns a new Yolo4ImportLibraryContext class instance.
   */
  static Yolo4ImportLibraryContextPtr create_yolo4_library_context();

  /**
   * @brief Factory method to create an empty library context of the
   * Ultralytics YOLO detection dataset layout.
   *
   * @return Returns a new UltralyticsDetectImportLibraryContext class
   * instance.
   */
  static UltralyticsDetectImportLibraryContextPtr
  create_ultralytics_detect_library_context();

  /**
   * @brief Factory method to create an empty library context of the
   * Ultralytics YOLO oriented bounding box dataset layout.
   *
   * @return Returns a new UltralyticsObbImportLibraryContext class instance.
   */
  static UltralyticsObbImportLibraryContextPtr
  create_ultralytics_obb_library_context();

  /**
   * @brief Factory method to create an empty library context of the
   * Ultralytics YOLO instance segmentation dataset layout.
   *
   * @return Returns a new UltralyticsSegmentImportLibraryContext class
   * instance.
   */
  static UltralyticsSegmentImportLibraryContextPtr
  create_ultralytics_segment_library_context();

  /**
   * @brief Factory method to create an empty library context of the COCO
   * object detection dataset layout.
   *
   * @return Returns a new CocoImportLibraryContext class instance.
   */
  static CocoImportLibraryContextPtr create_coco_library_context();

  /**
   * @brief Factory method to create an empty library context of the Pascal VOC
   * dataset layout.
   *
   * @return Returns a new PascalVocImportLibraryContext class instance.
   */
  static PascalVocImportLibraryContextPtr create_pascal_voc_library_context();

  /**
   * @brief Factory method to create an empty library context of the Create ML
   * object detection dataset layout.
   *
   * @return Returns a new CreateMLImportLibraryContext class instance.
   */
  static CreateMLImportLibraryContextPtr create_createml_library_context();

  /**
   * @brief Factory method to create an empty library context of the PyTorch
   * Vision dataset layout.
   *
   * @return Returns a new PyTorchImportLibraryContext class instance.
   */
  static PyTorchImportLibraryContextPtr create_pytorch_library_context();

  /**
   * @brief Creates the default library implementation.
   *
   * @return Returns the default library implementation. Currently returns
   * the LibMain class instance.
   */
  static ILibPtr create_default_lib();

  /**
   * @brief Creates and returns the appropriate ILib instance.
   *
   * @param ctx Filled LibraryContext to check for an appropriate ILib
   * instance.
   *
   * @return Returns a new ILib descendant or a nullptr in case of any error.
   */
  static ILibPtr create_library(LibraryContextPtr ctx);

  /**
   * @brief Creates the importer implementing the dataset layout of the given
   * context.
   *
   * @param ctx The LibraryContext descendant naming the wanted layout.
   *
   * @return Returns a new IImporter descendant, or a nullptr for an empty
   * context.
   */
  static IImporterPtr create_importer(const LibraryContextPtr& ctx);

  /**
   * @brief Factory method to create the image size reader the library ships
   * itself.
   *
   * The library decodes no image format of its own, which is why
   * LibraryContext::set_image_sizer() exists: an annotation of the internal
   * project format is a rectangle in the pixels of its image, so the layouts
   * which do not store those pixels ask their consumer to measure the picture.
   * When this build of the library found OpenCV it is able to do that itself,
   * and this method hands out such a reader.
   *
   * Handing it over to that setter is optional. An import left with an empty
   * slot falls back to this very reader on its own, so a consumer with nothing
   * better to offer may simply leave the slot alone. A reader the consumer does
   * set always wins over this one.
   *
   * @return Returns a new reader, or a nullptr when the library was built
   * without OpenCV. A nullptr means the import needs a reader of your own.
   */
  static IImageSizeFacilityPtr create_image_sizer();

  /**
   * @brief Reports the version of the library binary being used.
   *
   * @return Returns the library build version string.
   */
  static std::string library_version();

  /**
   * @brief Hands the logger instance of the project which uses this library
   * over to the library logging subsystem, so the whole binary logs into the
   * very same logger instead of the library keeping a silent own one.
   *
   * Call it before the first use of the library, usually right after the own
   * logging initialization. Any logger::ILogger implementation is accepted: a
   * project built from the cpp-app-template shares its default logger by
   * passing the LOG_REAL_LOGGER() value.
   *
   * @param realLogger The logger instance to adopt. A nullptr is ignored and
   * the previously adopted or the library own instance stays.
   */
  static void accept_real_logger(
      const std::shared_ptr<logger::ILogger>& realLogger);
};

}  // namespace ImagesAnnotatorDataImporters013

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYFACADE_CLASS_H
