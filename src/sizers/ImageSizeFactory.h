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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMAGESIZEFACTORY_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMAGESIZEFACTORY_H

#include "src/importers/ImportersAliases.h"

namespace iannotator::importers::sizers
{

/**
 * @brief Creates the image size reader the library ships itself, when it was
 * built with one.
 *
 * This is the one entry point to the built-in reader, and it is compiled into
 * every build of the library, with OpenCV and without. That is the whole point
 * of it: the caller asks the same question either way and branches on the
 * answer at the run time, instead of every call site repeating the build time
 * condition.
 *
 * @return Returns a new OpenCVImageSizer when the build found OpenCV, and a
 * nullptr when it did not. A nullptr is not an error - it only means the
 * consumer has to supply a reader of its own through
 * IADataImportersContext::set_image_sizer().
 */
IImageSizeFacilityPtr create_builtin_image_sizer();

}  // namespace iannotator::importers::sizers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMAGESIZEFACTORY_H
