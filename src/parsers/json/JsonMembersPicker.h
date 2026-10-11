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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONMEMBERSPICKER_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONMEMBERSPICKER_CLASS_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace iannotator::importers::parsers
{

/**
 * @brief Picks the values of the named members out of a JSON document which
 * is read a piece at a time, and leaves the rest of the document unparsed.
 *
 * The JsonParser reads a whole document into its tree, which is what a dataset
 * descriptor of a few megabytes asks for. The darknetxx dxxwjz files are no
 * such documents: the one of a trained network keeps the JSON text of every
 * weight of it, hundreds of megabytes, while an import wants the few members
 * of the annotations and of the class names alone. So the document is read
 * from a source a piece at a time (the gzip file stream of the zlib, say), the
 * values of no interest are stepped over by their brackets and their strings
 * alone, and the text of every wanted value is handed over as it is read, for
 * the JsonParser to read that value alone.
 *
 * A member is named by its path: the keys of the objects from the root one
 * down, so `{"network", "classes"}` is the `classes` member of the `network`
 * object of the root. The reading stops once every wanted value is handed
 * over, so nothing of the document after the last of them is read at all.
 *
 * The values stepped over are checked for no more than their brackets to
 * match and their strings to end: a malformed number or literal among them
 * goes unnoticed, while the JsonParser reads every value handed over in full.
 * The first member of a repeated key is the one picked.
 */
class JsonMembersPicker
{
 public:
  /// @brief Copies the next piece of the document into the buffer, the count
  /// of the copied bytes, 0 at the end of the document.
  using Source = std::function<std::size_t(char* buffer, std::size_t size)>;
  /// @brief The keys of the objects from the root one down to the member
  using Path = std::vector<std::string>;
  /// @brief Receives the JSON text of a picked value, returns false to stop
  /// the reading at once.
  using Receiver =
      std::function<bool(const Path& path, const std::string& text)>;

  explicit JsonMembersPicker(Source nsource);
  virtual ~JsonMembersPicker() = default;

  /**
   * @brief Reads the document up to the last of the wanted values, or up to
   * its end when not every one of them is there.
   *
   * @param wanted The paths of the wanted members.
   * @param receiver Receives every wanted value found, in the order of the
   * document.
   *
   * @return Returns false for the document which is no JSON object or is a
   * malformed one as far as it was read. A reading stopped by the receiver or
   * by the last wanted value is no failure.
   */
  bool pick(const std::vector<Path>& wanted, const Receiver& receiver);

 private:
  /// @brief How much of the document is asked of the source at a time
  inline static constexpr const std::size_t bufferSize = 65536U;
  /// @brief How deep the containers of a document may nest. The darknetxx
  /// documents nest below ten levels.
  inline static constexpr const std::size_t maxDepth = 64U;

  /// @brief Reads the members of the object under the cursor.
  bool read_object(Path& path);
  /// @brief Reads the value of the member of the path: hands it over, steps
  /// into it or steps over it.
  bool read_member_value(Path& path);
  bool read_key(std::string& key);

  /**
   * @brief Steps over the value under the cursor.
   *
   * @param text Out: the text of the value is appended to it, unless a
   * nullptr.
   */
  bool skip_value(std::string* text);
  bool skip_container(std::string* text);
  bool skip_string(std::string* text);
  bool skip_scalar(std::string* text);

  bool is_pending(const Path& path) const;
  bool leads_to_pending(const Path& path) const;

  void skip_blanks();
  /// @brief The byte under the cursor, -1 behind the end of the document
  int peek();
  bool consume(char expected);
  bool fill();

  Source source;
  std::vector<char> buffer;
  std::size_t pos{0U};
  std::size_t size{0U};
  bool ended{false};

  /// @brief The wanted members not found yet
  std::vector<Path> pending;
  const Receiver* receiver{nullptr};
  /// @brief Every wanted member is handed over, or the receiver asked to stop
  bool finished{false};
  std::size_t depth{0U};
};

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONMEMBERSPICKER_CLASS_H
