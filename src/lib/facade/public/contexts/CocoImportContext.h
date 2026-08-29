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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOIMPORTCONTEXT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOIMPORTCONTEXT_CLASS_H

#include <memory>

#include "../IADataImportersContext.h"
#include "../ImportersAPI.h"

namespace ImagesAnnotatorDataImporters014
{

/**
 * @brief The library context which reads the COCO object detection dataset -
 * the single JSON descriptor over a directory of pictures - back into image
 * records.
 *
 * The descriptor carries the size of every image it names, so this is one of
 * the layouts which need no IImageSizeFacility.
 *
 * The class carries no data of its own: instantiating it is what names the
 * wanted dataset layout, everything else is inherited from
 * IADataImportersContext.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API CocoImportContext : public IADataImportersContext
{
 public:
  using CocoImportContextPtr = std::shared_ptr<CocoImportContext>;
};

using CocoImportContextPtr = CocoImportContext::CocoImportContextPtr;

}  // namespace ImagesAnnotatorDataImporters014

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_COCOIMPORTCONTEXT_CLASS_H
