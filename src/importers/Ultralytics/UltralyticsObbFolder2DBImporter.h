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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSOBBFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSOBBFOLDER2DBIMPORTER_CLASS_H

#include <vector>

#include "src/importers/Ultralytics/UltralyticsFolder2DBImporter.h"

namespace iannotator::importers
{

/**
 * @brief The importer of the Ultralytics YOLO oriented bounding box dataset.
 *
 * The label file line of a rectangle is its class index followed by the four
 * corners of the box, clockwise from the top left one. The annotations
 * database knows axis aligned rectangles only, so what comes back of a box
 * some other tool wrote with a rotation angle is the upright rectangle which
 * holds it whole - and a box written by the sibling exporters library, whose
 * angle is always zero, comes back as the very rectangle it was drawn as.
 */
class UltralyticsObbFolder2DBImporter : public UltralyticsFolder2DBImporter
{
 public:
  virtual ~UltralyticsObbFolder2DBImporter() = default;
  UltralyticsObbFolder2DBImporter() = default;

 protected:
  virtual bool read_rectangle(const std::vector<double>& values,
                              NormalizedRect& nrect) const override;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSOBBFOLDER2DBIMPORTER_CLASS_H
