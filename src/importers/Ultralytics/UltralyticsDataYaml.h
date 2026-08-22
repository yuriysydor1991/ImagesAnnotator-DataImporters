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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSDATAYAML_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSDATAYAML_CLASS_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace iannotator::importers
{

/**
 * @brief The `data.yaml` descriptor every Ultralytics YOLO dataset is built
 * around, read back into the entries the import of such a dataset needs.
 *
 * The descriptor is the whole of the layout which is not a file on the disk:
 * the class names live in it - this layout has no `obj.names` file of the
 * darknet one - and so do the directories its images are taken from. Only the
 * `path`, `train`, `val` and `names` entries are read, since the rest of what
 * an Ultralytics release accepts there describes a training run and not a
 * dataset.
 *
 * The reading covers the shape the sibling exporters library writes - one
 * `<index>: '<name>'` line per class, indented under `names` - together with
 * the two list spellings of that same entry a dataset laid out by hand or by
 * another tool arrives in: the block one of `- <name>` lines and the inline
 * one of `[<name>, <name>]`. A name written in either quoting style comes back
 * unquoted, so a colon or a hash inside it stays a part of it.
 */
class UltralyticsDataYaml
{
 public:
  /// @brief The annotation name of every class index the descriptor declares,
  /// which is the index its label files carry
  using ClassNames = std::unordered_map<long long, std::string>;

  virtual ~UltralyticsDataYaml() = default;
  UltralyticsDataYaml() = default;

  /**
   * @brief Reads the descriptor out of the lines of its file.
   *
   * @param lines The descriptor lines, the empty ones already left out.
   *
   * @return Returns true when the descriptor declared at least one class name,
   * which is what a dataset of this layout can not be read without.
   */
  bool read(const std::vector<std::string>& lines);

  const ClassNames& get_class_names() const;

  /// @brief The `path` entry: the directory the two below are relative to, and
  /// an empty string for a descriptor which drops the entry to stay movable
  const std::string& get_root() const;

  /// @brief The `train` entry: the images directory of the training part
  const std::string& get_train() const;

  /// @brief The `val` entry: the images directory of the validation part,
  /// which the exporting side writes as the training one
  const std::string& get_val() const;

 private:
  inline static const std::string rootKey = "path";
  inline static const std::string trainKey = "train";
  inline static const std::string valKey = "val";
  inline static const std::string namesKey = "names";

  /**
   * @brief Reads one entry of the descriptor top level.
   *
   * @param line The already trimmed line.
   *
   * @return Returns true when the entry opened the block of the class names,
   * so that the indented lines behind it are read as those.
   */
  bool read_entry(const std::string& line);

  /**
   * @brief Reads one line of the indented class names block.
   *
   * @param line The already trimmed line.
   */
  void read_name_entry(const std::string& line);

  /**
   * @brief Reads the class names of an inline `[<name>, <name>]` list.
   *
   * @param list The list, its brackets included.
   */
  void read_names_list(const std::string& list);

  /// @brief Declares the given name under the next positional index, which is
  /// what the two list spellings number their names by
  void add_name(const std::string& name);

  /**
   * @brief Drops the comment of a line, the one inside a quoted value left
   * alone.
   *
   * @param line The line to strip.
   *
   * @return Returns the line up to its comment.
   */
  static std::string strip_comment(const std::string& line);

  /**
   * @brief Drops the quotes a value is written in.
   *
   * @param value The value to unquote.
   *
   * @return Returns the value itself: the doubled quote of the YAML single
   * quoted style is one quote of the name, and the backslash escapes of the
   * double quoted one are the symbols they name.
   */
  static std::string unquote(const std::string& value);

  ClassNames classNames;
  std::string root;
  std::string train;
  std::string val;
  /// @brief The index the next name of a list spelling is declared under
  long long nextIndex{0};
};

}  // namespace iannotator::importers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_ULTRALYTICSDATAYAML_CLASS_H
