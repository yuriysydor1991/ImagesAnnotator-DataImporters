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

#include "src/sizers/OpenCVImageSizer.h"

#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>

#include "src/log/log.h"

namespace iannotator::importers::sizers
{

bool OpenCVImageSizer::read_image_size(const std::string& imagePath, int& width,
                                       int& height)
{
  if (imagePath.empty()) {
    LOGE("Empty image path provided");
    return false;
  }

  // The whole picture is decoded to learn two numbers, since the OpenCV C++
  // API exposes no header only probe. The reduced IMREAD_ flags would decode
  // less but report the reduced size, and a box scaled by such a guess is a
  // box in the wrong place, so the full decode is what keeps the coordinates
  // exact.
  const cv::Mat image = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

  if (image.empty()) {
    LOGE("Fail to decode the image: " << imagePath);
    return false;
  }

  if (image.cols <= 0 || image.rows <= 0) {
    LOGE("The decoded image carries no area: " << imagePath);
    return false;
  }

  width = image.cols;
  height = image.rows;

  LOGT("Measured " << imagePath << " as " << width << "x" << height);

  return true;
}

IImageSizeFacilityPtr OpenCVImageSizer::clone()
{
  return std::make_shared<OpenCVImageSizer>();
}

}  // namespace iannotator::importers::sizers
