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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PASCALVOCFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PASCALVOCFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <vector>

#include "src/helpers/TypeHelper.h"
#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"
#include "src/parsers/xml/IXmlParser.h"
#include "src/parsers/xml/XmlNode.h"

namespace iannotator::importers
{

/**
 * @brief The Pascal VOC dataset to annotations importer class.
 *
 * Reads the devkit directory shape: one XML descriptor per image under
 * `Annotations`, the pictures under `JPEGImages` and the image lists under
 * `ImageSets/Main`. Every descriptor names its image, the size of it and one
 * `object` element per rectangle, whose `bndbox` holds the two corner points
 * that rectangle was drawn between, in the image own pixels.
 *
 * The lists are not read at all: they name a training and a validation part of
 * one and the same set - identical files, as the exporting side writes them -
 * while a project holds the images themselves and no split of them. So every
 * descriptor of the directory is read, whichever list happens to name it.
 *
 * A directory of the descriptors lying beside their pictures instead of under
 * the two sub-directories is read just as well, which is the shape
 * [LabelImg](https://github.com/HumanSignal/labelImg) saves its own work in.
 *
 * The `xmax` and the `ymax` of a box are read as the origin plus the size,
 * which is what the sibling exporters library writes. A descriptor of the
 * original VOC devkit, whose coordinates count from one instead of from zero,
 * therefore comes back as the very same box moved by a single pixel: both
 * corners carry that one, so the size of it stays.
 *
 * The `truncated`, the `difficult` and the `pose` of an object are read over.
 * The first is the mark of a box the image edge cut down, which the annotations
 * database holds nothing of, and the two others hold no geometry either - a
 * `difficult` rectangle is imported like every other one, since a project is
 * edited and not evaluated.
 */
class PascalVocFolder2DBImporter : public Folder2DBImporter,
                                   virtual public helpers::TypeHelper
{
 public:
  virtual ~PascalVocFolder2DBImporter() = default;
  PascalVocFolder2DBImporter() = default;

 protected:
  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

 private:
  using XmlNode = parsers::XmlNode;
  using Paths = std::vector<std::filesystem::path>;

  inline static const std::string annotationsRel = "Annotations";
  inline static const std::string imagesRel = "JPEGImages";
  inline static const std::string xmlExt = ".xml";
  inline static const std::string rootElement = "annotation";
  inline static const std::string objectElement = "object";
  inline static const std::string nameElement = "name";
  inline static const std::string fileNameElement = "filename";
  inline static const std::string folderElement = "folder";
  inline static const std::string sizeElement = "size";
  inline static const std::string boxElement = "bndbox";

  /**
   * @brief Lists the descriptors of the dataset.
   *
   * @param importDir The dataset root.
   *
   * @return Returns the `*.xml` files of the `Annotations` sub-directory, or
   * the ones of the import directory itself when there is no such
   * sub-directory, sorted by name so that one and the same directory imports
   * one and the same way twice.
   */
  static Paths collect_descriptors(const std::filesystem::path& importDir);

  /**
   * @brief Reads one descriptor into the record of the image it names.
   *
   * @param descriptorPath The descriptor to read.
   * @param importDir The dataset root its image is looked for under.
   * @param parser The XML reader of the running import.
   *
   * @return Returns the new record, or a nullptr for a descriptor which is no
   * readable one of this layout.
   */
  ImageRecordPtr read_descriptor(const std::filesystem::path& descriptorPath,
                                 const std::filesystem::path& importDir,
                                 parsers::IXmlParser& parser);

  /**
   * @brief Locates the image file one descriptor names.
   *
   * The directories are looked into in the order below, and the first one
   * really holding that file is the one the record points at: the `JPEGImages`
   * of the devkit shape, the one the `folder` element names, the directory of
   * the descriptor itself - which is where a flat dataset keeps its pictures -
   * and the dataset root.
   *
   * @param annotation The parsed descriptor.
   * @param descriptorPath The file it was read from.
   * @param importDir The dataset root.
   * @param fileName The image file name the descriptor carries.
   *
   * @return Returns the located path, or the first of those candidates when
   * none of them holds the file.
   */
  static std::filesystem::path resolve_image_path(
      const XmlNode& annotation, const std::filesystem::path& descriptorPath,
      const std::filesystem::path& importDir, const std::string& fileName);

  /// @brief Fills the record dimensions out of the `size` element, which the
  /// layout carries for every image
  static void read_size(const XmlNode& annotation, const ImageRecordPtr& ir);

  /// @brief Adds one rectangle per `object` element to the given record
  static void read_objects(const XmlNode& annotation, const ImageRecordPtr& ir);

  /**
   * @brief Turns one `object` element into a rectangle.
   *
   * @param object The element to read.
   *
   * @return Returns the new rectangle, or a nullptr for an element carrying no
   * whole box of an area.
   */
  static ImageRecordRectPtr read_object(const XmlNode& object);
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_PASCALVOCFOLDER2DBIMPORTER_CLASS_H
