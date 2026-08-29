#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H

#include <gmock/gmock.h>

#include <memory>

#include "IADataImportersContext.h"
#include "IADataImportersLib.h"

namespace iadi0impl
{

class LibMain : public ImagesAnnotatorDataImporters014::IADataImportersLib
{
 public:
  using LibMainPtr = std::shared_ptr<LibMain>;
  using IADataImportersContextPtr =
      ImagesAnnotatorDataImporters014::IADataImportersContextPtr;

  virtual ~LibMain() = default;
  LibMain() = default;

  MOCK_METHOD(bool, perform_import, (IADataImportersContextPtr ctx),
              (override));

  inline static LibMainPtr create() { return std::make_shared<LibMain>(); }
};

using LibMainPtr = LibMain::LibMainPtr;

}  // namespace iadi0impl

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_LIBRARYMAIN_CLASS_H
