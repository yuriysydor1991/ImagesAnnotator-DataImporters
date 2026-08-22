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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLNODE_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLNODE_CLASS_H

#include <memory>
#include <string>
#include <vector>

namespace iannotator::importers::parsers
{

/**
 * @brief One element of a parsed XML document.
 *
 * The library reads one dataset layout whose per image descriptor is an XML
 * file - the Pascal VOC one - and links no XML library of its own, exactly as
 * the sibling exporters library writes those very descriptors without one.
 * This is the tree such a descriptor is read into by XmlParser, and it carries
 * no more than that import asks of it: the element names, their nesting and
 * their character data.
 *
 * The attributes of an element are read over and dropped. Nothing of the
 * Pascal VOC layout is written as one, and an element which carries none needs
 * no room for them.
 *
 * A node owns its children, so releasing the root releases the whole document.
 * The lookups hand out observing pointers into that tree, which stay valid as
 * long as the root does.
 */
class XmlNode
{
 public:
  using XmlNodePtr = std::unique_ptr<XmlNode>;
  using Children = std::vector<XmlNodePtr>;

  virtual ~XmlNode() = default;
  explicit XmlNode(const std::string& nname = {});

  const std::string& get_name() const;

  /// @brief The character data of this very element, the nested elements left
  /// out and the entity references resolved
  const std::string& get_text() const;
  void append_text(const std::string& moreText);

  const Children& get_children() const;
  void add_child(XmlNodePtr child);

  /**
   * @brief Looks the first nested element of the given name up.
   *
   * @param name The element name to look for.
   *
   * @return Returns the observing pointer of that element, or a nullptr when
   * this one holds none such.
   */
  const XmlNode* find(const std::string& name) const;

  /**
   * @brief Reads the character data of the first nested element of the given
   * name.
   *
   * @param name The element name to look for.
   * @param value Out: the read text, left alone when there is no such element.
   *
   * @return Returns true when the element was there.
   */
  bool read_text(const std::string& name, std::string& value) const;

  /**
   * @brief Reads the character data of the first nested element of the given
   * name as a number.
   *
   * The value is read as a floating point one and not as an integer on
   * purpose: the coordinates of this layout are the pixels of an image, while
   * more than one tool writing the format spells them out with a fractional
   * part all the same.
   *
   * @param name The element name to look for.
   * @param value Out: the read number, left alone when there is no such
   * element or it holds no number.
   *
   * @return Returns true when the element was there and it held a number.
   */
  bool read_number(const std::string& name, double& value) const;

 private:
  std::string name;
  std::string text;
  Children children;
};

using XmlNodePtr = XmlNode::XmlNodePtr;

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_XMLNODE_CLASS_H
