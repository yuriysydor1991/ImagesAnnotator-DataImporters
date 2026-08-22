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

#include "src/parsers/xml/XmlNode.h"

#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <utility>

#include "src/log/log.h"

namespace iannotator::importers::parsers
{

XmlNode::XmlNode(const std::string& nname) : name{nname} {}

const std::string& XmlNode::get_name() const { return name; }

const std::string& XmlNode::get_text() const { return text; }

void XmlNode::append_text(const std::string& moreText) { text += moreText; }

const XmlNode::Children& XmlNode::get_children() const { return children; }

void XmlNode::add_child(XmlNodePtr child)
{
  children.emplace_back(std::move(child));
}

const XmlNode* XmlNode::find(const std::string& lookedFor) const
{
  for (const auto& child : children) {
    if (child != nullptr && child->get_name() == lookedFor) {
      return child.get();
    }
  }

  return nullptr;
}

bool XmlNode::read_text(const std::string& lookedFor, std::string& value) const
{
  const XmlNode* child = find(lookedFor);

  if (child == nullptr) {
    return false;
  }

  value = child->get_text();

  return true;
}

bool XmlNode::read_number(const std::string& lookedFor, double& value) const
{
  std::string token;

  if (!read_text(lookedFor, token)) {
    return false;
  }

  try {
    std::size_t consumed{0U};

    const double read = std::stod(token, &consumed);

    // The trailing blanks of an element written over more than one line are
    // no part of the number, while anything else behind it makes the whole
    // element something this import has no reading of.
    if (token.find_first_not_of(" \t\r\n", consumed) != std::string::npos) {
      LOGE("The " << lookedFor << " element holds no number: " << token);
      return false;
    }

    value = read;
  }
  catch (const std::exception& e) {
    LOGE("Fail to read the " << lookedFor << " element value " << token
                             << " reason: " << e.what());
    return false;
  }

  return true;
}

}  // namespace iannotator::importers::parsers
