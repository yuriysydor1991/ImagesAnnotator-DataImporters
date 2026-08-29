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

#include "IADataImportersContext.h"

#include <cstddef>
#include <memory>
#include <string>

namespace ImagesAnnotatorDataImporters014
{

const std::string& IADataImportersContext::get_import_path() const
{
  return import_path;
}

void IADataImportersContext::set_import_path(const std::string& newPath)
{
  import_path = newPath;
}

const IADataImportersContext::IAnnotationsDBPtr&
IADataImportersContext::get_db() const
{
  return db;
}

void IADataImportersContext::set_db(const IAnnotationsDBPtr& newDb)
{
  db = newDb;
}

const IImageSizeFacilityPtr& IADataImportersContext::get_image_sizer() const
{
  return imageSizer;
}

void IADataImportersContext::set_image_sizer(
    const IImageSizeFacilityPtr& newSizer)
{
  imageSizer = newSizer;
}

const IImporterPtr& IADataImportersContext::get_importer() const
{
  return importer;
}

void IADataImportersContext::set_importer(const IImporterPtr& newImporter)
{
  importer = newImporter;
}

const std::size_t& IADataImportersContext::get_imported_records() const
{
  return importedRecords;
}

void IADataImportersContext::set_imported_records(const std::size_t& newCount)
{
  importedRecords = newCount;
}

}  // namespace ImagesAnnotatorDataImporters014
