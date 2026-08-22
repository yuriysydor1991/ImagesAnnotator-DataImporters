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

#include "src/importers/Ultralytics/UltralyticsDetectFolder2DBImporter.h"

#include <cstddef>
#include <vector>

#include "src/log/log.h"

namespace iannotator::importers
{

namespace
{
/// @brief How many numbers the box of a detection line carries: the centre
/// and the size of it
constexpr const std::size_t boxFields = 4U;
}  // namespace

bool UltralyticsDetectFolder2DBImporter::read_rectangle(
    const std::vector<double>& values, NormalizedRect& nrect) const
{
  if (values.size() != boxFields) {
    LOGE("The detection label line carries "
         << values.size()
         << " numbers behind its class index instead of the four of a box");
    return false;
  }

  // The very halving the export performed, undone: the centre less half of
  // the size is the corner the box was drawn from.
  const double halfWidth = values[2U] / 2.0;
  const double halfHeight = values[3U] / 2.0;

  nrect.left = values[0U] - halfWidth;
  nrect.right = values[0U] + halfWidth;
  nrect.top = values[1U] - halfHeight;
  nrect.bottom = values[1U] + halfHeight;

  return true;
}

}  // namespace iannotator::importers
