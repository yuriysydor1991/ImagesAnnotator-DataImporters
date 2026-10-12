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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSALIASES_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSALIASES_H

#include <ImagesAnnotatorDataDrivers-0.14/IAnnotationsDB.h>
#include <ImagesAnnotatorDataDrivers-0.14/ImageRecord.h>
#include <ImagesAnnotatorDataDrivers-0.14/ImageRecordRect.h>

#include "IADataImportersContext.h"
#include "IImageSizeFacility.h"
#include "IImporter.h"

/**
 * @brief The images annotator importer namespace that holds all annotator
 * related importers classes.
 *
 * This is the implementation side of the library: nothing declared under it is
 * installed - a consuming project only ever sees the
 * ImagesAnnotatorDataImporters016 interfaces. The namespace root is the one the
 * sibling exporters library carries, so the two halves of the pair read alike
 * inside the ImagesAnnotator application they both came out of.
 */
namespace iannotator::importers
{

/**
 * @brief Pulls the installable interface names and the data drivers record
 * names into the implementation namespace, so that the importers code refers
 * to them unqualified.
 */
using ImagesAnnotatorDataImporters016::IADataImportersContext;
using ImagesAnnotatorDataImporters016::IADataImportersContextPtr;
using ImagesAnnotatorDataImporters016::IImageSizeFacility;
using ImagesAnnotatorDataImporters016::IImageSizeFacilityPtr;
using ImagesAnnotatorDataImporters016::IImporter;
using ImagesAnnotatorDataImporters016::IImporterPtr;

using ImagesAnnotatorDataDrivers014::IAnnotationsDB;
using ImagesAnnotatorDataDrivers014::IAnnotationsDBPtr;
using ImagesAnnotatorDataDrivers014::ImageRecord;
using ImagesAnnotatorDataDrivers014::ImageRecordPtr;
using ImagesAnnotatorDataDrivers014::ImageRecordRect;
using ImagesAnnotatorDataDrivers014::ImageRecordRectPtr;
using ImagesAnnotatorDataDrivers014::ImageRecordRectSet;
using ImagesAnnotatorDataDrivers014::ImageRecordsSet;

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSALIASES_H
