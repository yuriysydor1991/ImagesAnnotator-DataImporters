#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H

#include <gmock/gmock.h>

#include <memory>

#include "ILib.h"
#include "LibraryContext.h"

namespace iadi0impl
{

class LibMain : public ImagesAnnotatorDataImporters011::ILib
{
 public:
  using LibMainPtr = std::shared_ptr<LibMain>;
  using LibraryContextPtr = ImagesAnnotatorDataImporters011::LibraryContextPtr;

  virtual ~LibMain() = default;
  LibMain() = default;

  MOCK_METHOD(bool, perform_import, (LibraryContextPtr ctx), (override));

  inline static LibMainPtr create() { return std::make_shared<LibMain>(); }
};

using LibMainPtr = LibMain::LibMainPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
