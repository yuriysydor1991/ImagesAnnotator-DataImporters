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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMAGESIZEFACILITY_ABSTRACT_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMAGESIZEFACILITY_ABSTRACT_CLASS_H

#include <memory>
#include <string>

#include "ImportersAPI.h"

namespace ImagesAnnotatorDataImporters013
{

/**
 * @brief The image measuring service the consuming project supplies for the
 * imports which have to know how large an image is.
 *
 * The library decodes no image format of its own, and an annotation of the
 * internal project format is a rectangle in the pixels of the image it was
 * drawn over. A dataset which stores its boxes normalised - the YOLO v4 one
 * here - therefore can not be read back without the size of the image each box
 * belongs to, and the PyTorch Vision layout, whose whole annotation is the
 * cropped image itself, can not be read back without it either. So the imports
 * that need such a measurement ask their consumer to take it over whatever
 * imaging stack that project already links.
 *
 * The interface deliberately takes a path and not an image record: the record
 * is what the import is building, and its size fields are what this call
 * fills in.
 *
 * Current file is a target for the library header installation.
 */
class IADI_API IImageSizeFacility
{
 public:
  using IImageSizeFacilityPtr = std::shared_ptr<IImageSizeFacility>;

  virtual ~IImageSizeFacility() = default;
  IImageSizeFacility() = default;

  /**
   * @brief Reads the intrinsic pixel size of the given image file.
   *
   * @param imagePath The filesystem path of the image to measure.
   * @param width Out: the image width in pixels.
   * @param height Out: the image height in pixels.
   *
   * @return Should return true when both dimensions were read. Returning false
   * for an unreadable or an unsupported file is not an error of its own: the
   * import logs that one image and carries on with the rest.
   */
  virtual bool read_image_size(const std::string& imagePath, int& width,
                               int& height) = 0;

  /**
   * @brief Produces a copy sharing no mutable decoding state with this one.
   *
   * @return Should return a new usable instance.
   */
  virtual IImageSizeFacilityPtr clone() = 0;
};

using IImageSizeFacilityPtr = IImageSizeFacility::IImageSizeFacilityPtr;

}  // namespace ImagesAnnotatorDataImporters013

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IIMAGESIZEFACILITY_ABSTRACT_CLASS_H
