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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONVALUE_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONVALUE_CLASS_H

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace iannotator::importers::parsers
{

/**
 * @brief One node of a parsed JSON document.
 *
 * The library reads two dataset layouts whose whole descriptor is a JSON file
 * - the COCO one and the Create ML one - and links no JSON library of its
 * own, exactly as the sibling exporters library writes those very descriptors
 * without one. This is the tree such a descriptor is read into by JsonParser,
 * and it carries no more than those two imports ask of it.
 *
 * A node owns its children, so releasing the root releases the whole
 * document. The lookups hand out observing pointers into that tree, which stay
 * valid as long as the root does.
 */
class JsonValue
{
 public:
  using JsonValuePtr = std::unique_ptr<JsonValue>;
  using Array = std::vector<JsonValuePtr>;
  using Member = std::pair<std::string, JsonValuePtr>;
  /// @brief The members in the very order the document wrote them. A vector
  /// and not a map: a descriptor object of this format carries a handful of
  /// keys, over which a walk is cheaper than the hashing of a lookup.
  using Object = std::vector<Member>;

  /// @brief The JSON types. The parser tells a number from a string, and the
  /// two imports never ask a null or a boolean for anything but its presence.
  enum class Kind
  {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
  };

  virtual ~JsonValue() = default;
  explicit JsonValue(const Kind& nkind = Kind::Null);

  const Kind& get_kind() const;

  bool is_array() const;
  bool is_object() const;
  bool is_number() const;
  bool is_string() const;

  /// @brief The text of a string node, empty for every other kind
  const std::string& get_text() const;
  void set_text(const std::string& newText);

  /// @brief The value of a number node, zero for every other kind
  const double& get_number() const;
  void set_number(const double& newNumber);

  /// @brief The elements of an array node, empty for every other kind
  const Array& get_items() const;
  void add_item(JsonValuePtr item);

  /// @brief The members of an object node, empty for every other kind
  const Object& get_members() const;
  void add_member(const std::string& key, JsonValuePtr value);

  /**
   * @brief Looks the named member of an object node up.
   *
   * @param key The member name to look for.
   *
   * @return Returns the observing pointer of that member value, or a nullptr
   * when this node is no object or holds no such member.
   */
  const JsonValue* find(const std::string& key) const;

  /**
   * @brief Reads the named member of an object node as a number.
   *
   * @param key The member name to look for.
   * @param value Out: the read number, left alone when there is none.
   *
   * @return Returns true when the member was there and it was a number.
   */
  bool read_number(const std::string& key, double& value) const;

  /**
   * @brief Reads the named member of an object node as a string.
   *
   * @param key The member name to look for.
   * @param value Out: the read text, left alone when there is none.
   *
   * @return Returns true when the member was there and it was a string.
   */
  bool read_text(const std::string& key, std::string& value) const;

  /**
   * @brief Reads the named member of an object node as an array of numbers.
   *
   * @param key The member name to look for.
   * @param values Out: the read numbers, left alone when the member is no
   * array or holds anything which is not a number.
   *
   * @return Returns true when every element of that array was a number.
   */
  bool read_numbers(const std::string& key, std::vector<double>& values) const;

 private:
  Kind kind;
  std::string text;
  double number{0.0};
  Array items;
  Object members;
};

using JsonValuePtr = JsonValue::JsonValuePtr;

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_JSONVALUE_CLASS_H
