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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYCONTEXT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYCONTEXT_CLASS_H

#include <ImagesAnnotatorDataDrivers-0.11/IAnnotationsDB.h>

#include <cstddef>
#include <memory>
#include <string>

#include "IImageSizeFacility.h"
#include "IImporter.h"
#include "ImportersAPI.h"

namespace ImagesAnnotatorDataImporters011
{

/**
 * @brief The library context class designed to pass data in and out of
 * the library underlying implementation.
 *
 * The very same instance drives both entry points of the library: the one
 * shot ILib::perform_import and the IImporter::import_db of an importer built
 * by hand through the LibraryFacade factory methods.
 *
 * The dataset layout to read is named by the descendant instantiated. What
 * every layout needs is held here; a descendant adds only what its own layout
 * asks for. See PlainTxtImportLibraryContext, Yolo4ImportLibraryContext and
 * PyTorchImportLibraryContext. This class itself names no layout, so an import
 * driven by it finds no importer.
 *
 * The image size reader sits here rather than in a single descendant, unlike
 * the image cropper of the sibling exporters library: two of the three layouts
 * read here need a measurement, and the third fills the image dimensions of the
 * produced records with it when it is there.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API LibraryContext
{
 public:
  using LibraryContextPtr = std::shared_ptr<LibraryContext>;
  using IAnnotationsDBPtr = ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr;

  virtual ~LibraryContext() = default;
  LibraryContext() = default;

  /// @brief In: the source directory of the import, mandatory
  const std::string& get_import_path() const;
  void set_import_path(const std::string& newPath);

  /// @brief In-out: the annotations database the recovered records are merged
  /// into, mandatory. An already held record is kept as it is, so importing
  /// one and the same dataset twice adds it once. Obtain an empty one from
  /// ImagesAnnotatorDataDrivers011::LibraryFacade::create_annotations_db(), or
  /// hand over the database of the project being edited to import into it.
  const IAnnotationsDBPtr& get_db() const;
  void set_db(const IAnnotationsDBPtr& newDb);

  /// @brief In: the image measuring instance. Mandatory for the layouts which
  /// can not be read without it - the YOLO v4 one, whose boxes are normalised,
  /// and the PyTorch Vision one, whose annotation is the cropped image itself
  /// - and optional for the rest, where it only fills in the image dimensions
  /// of the produced records. A build of the library which found OpenCV falls
  /// back to a reader of its own when this slot is left empty, see
  /// LibraryFacade::create_image_sizer().
  const IImageSizeFacilityPtr& get_image_sizer() const;
  void set_image_sizer(const IImageSizeFacilityPtr& newSizer);

  /// @brief Out: the importer instance the last ILib::perform_import ran
  const IImporterPtr& get_importer() const;
  void set_importer(const IImporterPtr& newImporter);

  /// @brief Out: how many image records the last import handed to the database
  const std::size_t& get_imported_records() const;
  void set_imported_records(const std::size_t& newCount);

 private:
  std::string import_path;
  IAnnotationsDBPtr db;
  IImageSizeFacilityPtr imageSizer;
  IImporterPtr importer;
  std::size_t importedRecords{0};
};

using LibraryContextPtr = LibraryContext::LibraryContextPtr;

}  // namespace ImagesAnnotatorDataImporters011

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYCONTEXT_CLASS_H
