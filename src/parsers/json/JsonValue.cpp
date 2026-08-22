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

#include "src/parsers/json/JsonValue.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace iannotator::importers::parsers
{

JsonValue::JsonValue(const Kind& nkind) : kind{nkind} {}

const JsonValue::Kind& JsonValue::get_kind() const { return kind; }

bool JsonValue::is_array() const { return kind == Kind::Array; }

bool JsonValue::is_object() const { return kind == Kind::Object; }

bool JsonValue::is_number() const { return kind == Kind::Number; }

bool JsonValue::is_string() const { return kind == Kind::String; }

const std::string& JsonValue::get_text() const { return text; }

void JsonValue::set_text(const std::string& newText) { text = newText; }

const double& JsonValue::get_number() const { return number; }

void JsonValue::set_number(const double& newNumber) { number = newNumber; }

const JsonValue::Array& JsonValue::get_items() const { return items; }

void JsonValue::add_item(JsonValuePtr item)
{
  items.emplace_back(std::move(item));
}

const JsonValue::Object& JsonValue::get_members() const { return members; }

void JsonValue::add_member(const std::string& key, JsonValuePtr value)
{
  members.emplace_back(key, std::move(value));
}

const JsonValue* JsonValue::find(const std::string& key) const
{
  for (const auto& member : members) {
    if (member.first == key) {
      return member.second.get();
    }
  }

  return nullptr;
}

bool JsonValue::read_number(const std::string& key, double& value) const
{
  const JsonValue* member = find(key);

  if (member == nullptr || !member->is_number()) {
    return false;
  }

  value = member->get_number();

  return true;
}

bool JsonValue::read_text(const std::string& key, std::string& value) const
{
  const JsonValue* member = find(key);

  if (member == nullptr || !member->is_string()) {
    return false;
  }

  value = member->get_text();

  return true;
}

bool JsonValue::read_numbers(const std::string& key,
                             std::vector<double>& values) const
{
  const JsonValue* member = find(key);

  if (member == nullptr || !member->is_array()) {
    return false;
  }

  std::vector<double> read;

  read.reserve(member->get_items().size());

  for (const auto& item : member->get_items()) {
    if (item == nullptr || !item->is_number()) {
      return false;
    }

    read.emplace_back(item->get_number());
  }

  values.swap(read);

  return true;
}

}  // namespace iannotator::importers::parsers
