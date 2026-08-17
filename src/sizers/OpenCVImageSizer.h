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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_OPENCVIMAGESIZER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_OPENCVIMAGESIZER_CLASS_H

#include <string>

#include "src/importers/ImportersAliases.h"

namespace iannotator::importers::sizers
{

/**
 * @brief The IImageSizeFacility implementation the library ships itself, built
 * on OpenCV.
 *
 * The library decodes no image format of its own, which is why the imports
 * which have to know how large an image is ask their consumer for a reader.
 * This class is the one exception: when the build found OpenCV, the library is
 * able to answer that request itself, so a consumer without an imaging stack
 * of its own still gets those imports. It is compiled only then - see
 * create_builtin_image_sizer(), which hands out a nullptr in a build without
 * OpenCV.
 *
 * A reader the consumer sets through LibraryContext::set_image_sizer() always
 * wins over this one: a project that already decodes images its own way keeps
 * doing so, and this is only what fills an empty slot.
 *
 * Nothing declared here is installed. The class is reached through the
 * abstract IImageSizeFacility exactly like a consumer supplied reader, so no
 * OpenCV type reaches a public header and no consuming project needs OpenCV of
 * its own.
 */
class OpenCVImageSizer : virtual public IImageSizeFacility
{
 public:
  virtual ~OpenCVImageSizer() = default;
  OpenCVImageSizer() = default;

  /**
   * @brief Reads the intrinsic pixel size of the given image file with OpenCV.
   *
   * @param imagePath The filesystem path of the image to measure.
   * @param width Out: the image width in pixels, left alone on a failure.
   * @param height Out: the image height in pixels, left alone on a failure.
   *
   * @return Returns true when the file was decoded and both dimensions read.
   */
  bool read_image_size(const std::string& imagePath, int& width,
                       int& height) override;

  /**
   * @brief Produces a copy of this reader.
   *
   * @return Returns a new usable instance. The class holds no decoding state
   * between the calls, so the copy is a plain new instance.
   */
  IImageSizeFacilityPtr clone() override;
};

}  // namespace iannotator::importers::sizers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_OPENCVIMAGESIZER_CLASS_H
