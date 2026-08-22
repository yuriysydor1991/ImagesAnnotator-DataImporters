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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_TYPE_HELPER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_TYPE_HELPER_CLASS_H

#include <ImagesAnnotatorDataDrivers-0.11/ImageRecord.h>
#include <ImagesAnnotatorDataDrivers-0.11/ImageRecordRect.h>

#include <cmath>

#include "src/helpers/IHelper.h"

namespace iannotator::importers::helpers
{

/**
 * @brief The helper class to shorted the static_cast<> expressions in the code
 */
class TypeHelper : virtual public IHelper
{
 public:
  using ImageRecordsSet = ImagesAnnotatorDataDrivers011::ImageRecordsSet;
  using ImageRecordPtr = ImagesAnnotatorDataDrivers011::ImageRecordPtr;
  using ImageRecordRect = ImagesAnnotatorDataDrivers011::ImageRecordRect;
  using ImageRecordRectSet = ImagesAnnotatorDataDrivers011::ImageRecordRectSet;
  using ImageRecordRectPtr = ImagesAnnotatorDataDrivers011::ImageRecordRectPtr;

  virtual ~TypeHelper() = default;
  TypeHelper() = default;

  template <class Ntype>
  inline static double toD(const Ntype& val)
  {
    return static_cast<double>(val);
  }

  /**
   * @brief Rounds a number a dataset descriptor holds onto the image pixel it
   * names.
   *
   * Every layout read here writes at least some of its coordinates as
   * fractions - the normalised YOLO ones, the halved Create ML centres - while
   * an ImageRecordRect is drawn in whole pixels, so the rounding is the very
   * last step of every one of those readings.
   *
   * @param val The number to round.
   *
   * @return Returns the nearest pixel coordinate.
   */
  inline static int toPixels(const double& val)
  {
    return static_cast<int>(std::lround(val));
  }
};

}  // namespace iannotator::importers::helpers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_TYPE_HELPER_CLASS_H
