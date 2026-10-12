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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2IMPORTCONTEXT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2IMPORTCONTEXT_CLASS_H

#include <memory>

#include "../IADataImportersContext.h"
#include "../ImportersAPI.h"

namespace ImagesAnnotatorDataImporters015
{

/**
 * @brief The library context which reads the darknetxx training directory of
 * the dxxwjz2 network back into image records: the YOLO v4 directory with the
 * `cfg/yolov4-obj.dxxwjz2` network to train beside its cfg file.
 *
 * The directory is read the way the Yolo4ImportContext reads it, while the
 * class names the label files number by come out of the network
 * (https://github.com/yuriysydor1991/darknetxx keeps them in its dxxwjz2
 * files) once the directory holds no `data/obj.names` file, the darknetxx way.
 * Requires an IImageSizeFacility unless the library was built with its own,
 * since the layout stores its boxes normalised.
 *
 * The class carries no data of its own: instantiating it is what names the
 * wanted dataset layout, everything else is inherited from
 * IADataImportersContext.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API Dxxwjz2ImportContext : public IADataImportersContext
{
 public:
  using Dxxwjz2ImportContextPtr = std::shared_ptr<Dxxwjz2ImportContext>;
};

using Dxxwjz2ImportContextPtr = Dxxwjz2ImportContext::Dxxwjz2ImportContextPtr;

}  // namespace ImagesAnnotatorDataImporters015

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2IMPORTCONTEXT_CLASS_H
