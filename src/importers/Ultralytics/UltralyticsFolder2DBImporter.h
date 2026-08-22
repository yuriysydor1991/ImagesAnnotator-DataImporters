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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSFOLDER2DBIMPORTER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSFOLDER2DBIMPORTER_CLASS_H

#include <filesystem>
#include <string>
#include <vector>

#include "src/helpers/TypeHelper.h"
#include "src/importers/Folder2DBImporter.h"
#include "src/importers/ImportersAliases.h"
#include "src/importers/Ultralytics/UltralyticsDataYaml.h"

namespace iannotator::importers
{

/**
 * @brief The directory layout every Ultralytics YOLO dataset shares, read back
 * into the annotations it was written out of.
 *
 * The layout is the one YOLO v5 introduced and every Ultralytics release since
 * - v8, v11 and the ones after them - kept: the `data.yaml` descriptor naming
 * the classes and the image directories, the pictures under `images/train` and
 * one `.txt` label file per picture under `labels/train`, paired with it by the
 * very path - the `images` element of it swapped for `labels` and the extension
 * for `.txt`.
 *
 * What the trained task changes is the single label file line of a rectangle,
 * which is why that line is the one thing this class leaves to its descendants.
 * See UltralyticsDetectFolder2DBImporter, UltralyticsObbFolder2DBImporter and
 * UltralyticsSegmentFolder2DBImporter.
 *
 * All three lines carry their numbers normalised by the size of the image, so
 * none of the three layouts can be read back without measuring the pictures.
 * A coordinate outside the `0..1` range - which an Ultralytics release refuses
 * an image over - is cut down to the image, the way the exporting side cuts a
 * rectangle reaching over an edge.
 *
 * The darknet directory of Yolo4Folder2DBImporter is a different layout
 * altogether: it names its classes in `data/obj.names` instead of a descriptor,
 * lists its images in `data/train.txt` instead of holding them in a directory
 * of their own, and keeps a label file beside its image instead of under a
 * directory of its own.
 */
class UltralyticsFolder2DBImporter : public Folder2DBImporter,
                                     virtual public helpers::TypeHelper
{
 public:
  virtual ~UltralyticsFolder2DBImporter() = default;
  UltralyticsFolder2DBImporter() = default;

 protected:
  /**
   * @brief One rectangle of a label file line: the box in the `0..1` range of
   * the image it was drawn over, which is the form every one of these layouts
   * writes its geometry in.
   */
  struct NormalizedRect
  {
    double left{0.0};
    double top{0.0};
    double right{0.0};
    double bottom{0.0};
  };

  virtual bool read_dataset(const std::filesystem::path& importDir,
                            ImageRecordsSet& records) override;

  /// @brief Every line is normalised, so the layout is unreadable without a
  /// measurement of every image the boxes of it belong to.
  virtual bool needs_image_sizer() const override;

  /**
   * @brief Reads the numbers of one label file line, the class index apart, as
   * the box the implemented task writes them for.
   *
   * @param values The numbers of the line behind its class index.
   * @param nrect Out: the box they describe, left alone on a failure.
   *
   * @return Should return false when the count of the numbers is none the
   * implemented task ever writes, so that the line is skipped instead of
   * placing a box nobody drew.
   */
  virtual bool read_rectangle(const std::vector<double>& values,
                              NormalizedRect& nrect) const = 0;

  /**
   * @brief Reads the given point coordinates as the box which holds them all.
   *
   * The four corners of an oriented bounding box and the points of a
   * segmentation polygon are both a ring of `x y` pairs, and the rectangle the
   * annotations database holds is what encloses such a ring.
   *
   * @param values The `x y` pairs of the ring.
   * @param nrect Out: the enclosing box, left alone on a failure.
   *
   * @return Returns true when the values were a whole number of pairs.
   */
  static bool read_corners(const std::vector<double>& values,
                           NormalizedRect& nrect);

 private:
  using Paths = std::vector<std::filesystem::path>;
  using ClassNames = UltralyticsDataYaml::ClassNames;

  inline static const std::string defaultDescriptor = "data.yaml";
  inline static const std::string yamlExt = ".yaml";
  inline static const std::string ymlExt = ".yml";
  inline static const std::string imagesRel = "images";
  inline static const std::string labelsRel = "labels";
  inline static const std::string trainRel = "images/train";
  inline static const std::string txtExt = ".txt";

  /**
   * @brief Lists the files the descriptor may be found in, most expected
   * first: the name the sibling exporters library writes, then any other YAML
   * file of the directory - which is how the public datasets of the format
   * name theirs.
   *
   * @param importDir The dataset root.
   *
   * @return Returns the candidate paths in the order they are to be tried.
   */
  static Paths descriptor_candidates(const std::filesystem::path& importDir);

  /**
   * @brief Reads the first of those candidates which describes a dataset.
   *
   * @param importDir The dataset root.
   * @param descriptor Out: the read descriptor.
   *
   * @return Returns true when one of the candidates declared class names.
   */
  bool read_descriptor(const std::filesystem::path& importDir,
                       UltralyticsDataYaml& descriptor) const;

  /**
   * @brief Locates the image directories of the dataset.
   *
   * The `train` and the `val` entry of the descriptor are followed, the import
   * path taken as the dataset root before the `path` entry of that descriptor:
   * a directory which was moved carries a `path` naming where it no longer is,
   * and dropping that very line is what the exporting side documents as the way
   * to move it.
   *
   * @param importDir The dataset root.
   * @param descriptor The read descriptor.
   *
   * @return Returns the existing directories, each of them once, in the order
   * the descriptor names them.
   */
  static Paths resolve_image_dirs(const std::filesystem::path& importDir,
                                  const UltralyticsDataYaml& descriptor);

  /**
   * @brief Adds the given directory to the list when it is one and is not
   * named there yet.
   *
   * @param dirPath The directory to add.
   * @param dirs In-out: the list to add it to.
   *
   * @return Returns true when the list names that directory afterwards.
   */
  static bool add_image_dir(const std::filesystem::path& dirPath, Paths& dirs);

  /**
   * @brief Lists the images of one such directory.
   *
   * @param dirPath The directory to walk.
   *
   * @return Returns its files, the label ones left out, sorted by name so that
   * one and the same directory imports one and the same way twice.
   */
  static Paths collect_images(const std::filesystem::path& dirPath);

  /**
   * @brief Builds the path of the label file of the given image.
   *
   * @param imagePath The image the labels are wanted of.
   *
   * @return Returns that path: the last `images` element of the image path
   * swapped for `labels` and the extension for `.txt`, or - for an image lying
   * under no such element at all - the very same directory the image is in.
   */
  static std::filesystem::path label_path_of(
      const std::filesystem::path& imagePath);

  /**
   * @brief Reads one image of the dataset together with its label file.
   *
   * @param imagePath The image to read.
   * @param classes The class names the descriptor declares.
   *
   * @return Returns the new record, or a nullptr for an image which could not
   * be measured - and whose normalised boxes are therefore unplaceable.
   */
  ImageRecordPtr read_image(const std::filesystem::path& imagePath,
                            const ClassNames& classes);

  /// @brief Adds one rectangle per label file line to the given record
  void read_label_file(const std::filesystem::path& labelPath,
                       const ClassNames& classes,
                       const ImageRecordPtr& ir) const;

  /**
   * @brief Turns one label file line into a rectangle.
   *
   * @param line The line to read.
   * @param classes The class names the descriptor declares.
   * @param ir The record of the image, whose measured size the box is
   * multiplied back by.
   *
   * @return Returns the new rectangle, or a nullptr for a line which is none
   * of this layout.
   */
  ImageRecordRectPtr read_label_line(const std::string& line,
                                     const ClassNames& classes,
                                     const ImageRecordPtr& ir) const;
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSFOLDER2DBIMPORTER_CLASS_H
