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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PIMPORTCONTEXT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PIMPORTCONTEXT_CLASS_H

#include <memory>

#include "../IADataImportersContext.h"
#include "../ImportersAPI.h"

namespace ImagesAnnotatorDataImporters014
{

/**
 * @brief The library context which reads the darknetxx dxxwjz2p training
 * project back into image records: the annotations member of the project file
 * - the ImagesAnnotator project structure of the folders of the images, their
 * sizes and the boxes in the pixels of the images - with the image paths
 * relative to the folder of the file.
 *
 * The dxxwjz2p file of the darknetxx
 * (https://github.com/yuriysydor1991/darknetxx) is the gzip compressed JSON
 * document of the whole YOLO v4 training project but the images. The import
 * path names the project file itself, or the directory holding it - the
 * `yolov4-obj.dxxwjz2p` of the sibling exporters library first, the first
 * `*.dxxwjz2p` file by the name otherwise. The project keeps the size of every
 * image of the boxes, so the layout needs no IImageSizeFacility, which fills
 * in the size of the images of none when it is there.
 *
 * The class carries no data of its own: instantiating it is what names the
 * wanted dataset layout, everything else is inherited from
 * IADataImportersContext.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API Dxxwjz2pImportContext : public IADataImportersContext
{
 public:
  using Dxxwjz2pImportContextPtr = std::shared_ptr<Dxxwjz2pImportContext>;
};

using Dxxwjz2pImportContextPtr =
    Dxxwjz2pImportContext::Dxxwjz2pImportContextPtr;

}  // namespace ImagesAnnotatorDataImporters014

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DXXWJZ2PIMPORTCONTEXT_CLASS_H
