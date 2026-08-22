#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "src/parsers/json/JsonParser.h"

using namespace testing;
using iannotator::importers::parsers::JsonParser;
using iannotator::importers::parsers::JsonValue;
using iannotator::importers::parsers::JsonValuePtr;

namespace
{

class UTEST_JsonParser : public Test
{
 public:
  JsonParser parser;

  JsonValuePtr parse(const std::string& document)
  {
    return parser.parse(document);
  }
};

}  // namespace

// The five keys of a COCO descriptor, in the shape the sibling exporters
// library writes them.
TEST_F(UTEST_JsonParser, reads_an_object_of_every_value_kind)
{
  auto root = parse(
      R"({"info": {"description": "a dataset"}, "licenses": [],
          "images": [{"id": 1, "width": 640, "height": 400}],
          "iscrowd": 0, "empty": null, "flag": true})");

  ASSERT_NE(root, nullptr);
  ASSERT_TRUE(root->is_object());

  const JsonValue* info = root->find("info");

  ASSERT_NE(info, nullptr);

  std::string description;

  EXPECT_TRUE(info->read_text("description", description));
  EXPECT_EQ(description, "a dataset");

  ASSERT_NE(root->find("licenses"), nullptr);
  EXPECT_TRUE(root->find("licenses")->is_array());
  EXPECT_TRUE(root->find("licenses")->get_items().empty());

  const JsonValue* images = root->find("images");

  ASSERT_NE(images, nullptr);
  ASSERT_TRUE(images->is_array());
  ASSERT_EQ(images->get_items().size(), 1U);

  double width{0.0};

  EXPECT_TRUE(images->get_items().front()->read_number("width", width));
  EXPECT_DOUBLE_EQ(width, 640.0);

  ASSERT_NE(root->find("empty"), nullptr);
  EXPECT_EQ(root->find("empty")->get_kind(), JsonValue::Kind::Null);
  ASSERT_NE(root->find("flag"), nullptr);
  EXPECT_EQ(root->find("flag")->get_kind(), JsonValue::Kind::Boolean);
}

TEST_F(UTEST_JsonParser, hands_out_no_member_of_another_name)
{
  auto root = parse(R"({"id": 7})");

  ASSERT_NE(root, nullptr);

  EXPECT_EQ(root->find("absent"), nullptr);

  double value{0.0};
  std::string text;

  EXPECT_FALSE(root->read_number("absent", value));
  EXPECT_FALSE(root->read_text("id", text));
}

TEST_F(UTEST_JsonParser, reads_the_box_of_an_annotation_as_its_numbers)
{
  auto root = parse(R"({"bbox": [50, 20.5, 100, -40], "area": 4000})");

  ASSERT_NE(root, nullptr);

  std::vector<double> box;

  ASSERT_TRUE(root->read_numbers("bbox", box));
  ASSERT_EQ(box.size(), 4U);
  EXPECT_DOUBLE_EQ(box[0U], 50.0);
  EXPECT_DOUBLE_EQ(box[1U], 20.5);
  EXPECT_DOUBLE_EQ(box[3U], -40.0);
}

TEST_F(UTEST_JsonParser,
       reads_no_numbers_out_of_an_array_holding_something_else)
{
  auto root = parse(R"({"bbox": [50, "20", 100, 40]})");

  ASSERT_NE(root, nullptr);

  std::vector<double> box;

  EXPECT_FALSE(root->read_numbers("bbox", box));
  EXPECT_TRUE(box.empty());
}

TEST_F(UTEST_JsonParser, reads_the_numbers_of_every_json_spelling)
{
  auto root = parse(R"({"a": -1, "b": 0.125, "c": 1e3, "d": 2.5E-2})");

  ASSERT_NE(root, nullptr);

  double value{0.0};

  ASSERT_TRUE(root->read_number("a", value));
  EXPECT_DOUBLE_EQ(value, -1.0);
  ASSERT_TRUE(root->read_number("b", value));
  EXPECT_DOUBLE_EQ(value, 0.125);
  ASSERT_TRUE(root->read_number("c", value));
  EXPECT_DOUBLE_EQ(value, 1000.0);
  ASSERT_TRUE(root->read_number("d", value));
  EXPECT_DOUBLE_EQ(value, 0.025);
}

// An annotation name is user text, and both descriptor layouts escape it.
TEST_F(UTEST_JsonParser, resolves_the_string_escapes)
{
  auto root = parse(R"({"name": "a \"quoted\" \\ name\nof\ttext \u0041"})");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "a \"quoted\" \\ name\nof\ttext A");
}

TEST_F(UTEST_JsonParser, resolves_a_surrogate_pair_into_one_character)
{
  auto root = parse(R"({"name": "\uD83D\uDE00"})");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "\xF0\x9F\x98\x80");
}

TEST_F(UTEST_JsonParser, reads_a_code_point_outside_ascii_as_utf8)
{
  auto root = parse(R"({"name": "\u00E4\u0500"})");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "\xC3\xA4\xD4\x80");
}

TEST_F(UTEST_JsonParser, reads_a_high_surrogate_without_its_low_half_as_none)
{
  EXPECT_EQ(parse(R"({"name": "\uD83D"})"), nullptr);
  EXPECT_EQ(parse(R"({"name": "\uDE00"})"), nullptr);
}

TEST_F(UTEST_JsonParser, reads_the_create_ml_descriptor_array)
{
  auto root = parse(R"([
    {"imagefilename": "street.png", "annotation": [
      {"label": "dog", "coordinates": {"x": 100, "y": 40, "width": 100,
                                       "height": 40}}
    ]}
  ])");

  ASSERT_NE(root, nullptr);
  ASSERT_TRUE(root->is_array());
  ASSERT_EQ(root->get_items().size(), 1U);

  const JsonValue* entry = root->get_items().front().get();

  std::string fileName;

  ASSERT_TRUE(entry->read_text("imagefilename", fileName));
  EXPECT_EQ(fileName, "street.png");

  const JsonValue* annotations = entry->find("annotation");

  ASSERT_NE(annotations, nullptr);
  ASSERT_EQ(annotations->get_items().size(), 1U);

  const JsonValue* coordinates =
      annotations->get_items().front()->find("coordinates");

  ASSERT_NE(coordinates, nullptr);

  double centre{0.0};

  ASSERT_TRUE(coordinates->read_number("x", centre));
  EXPECT_DOUBLE_EQ(centre, 100.0);
}

TEST_F(UTEST_JsonParser, keeps_the_members_of_one_name_in_their_order)
{
  auto root = parse(R"({"id": 1, "id": 2})");

  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->get_members().size(), 2U);

  double value{0.0};

  ASSERT_TRUE(root->read_number("id", value));
  EXPECT_DOUBLE_EQ(value, 1.0);
}

TEST_F(UTEST_JsonParser, reads_the_blanks_around_every_token_over)
{
  auto root = parse("  {\n\t\"id\"  :  [ 1 , 2 ]\r\n}  ");

  ASSERT_NE(root, nullptr);

  std::vector<double> values;

  ASSERT_TRUE(root->read_numbers("id", values));
  EXPECT_EQ(values.size(), 2U);
}

TEST_F(UTEST_JsonParser, refuses_a_malformed_document)
{
  EXPECT_EQ(parse(""), nullptr);
  EXPECT_EQ(parse("{"), nullptr);
  EXPECT_EQ(parse("{}}"), nullptr);
  EXPECT_EQ(parse(R"({"a": 1,})"), nullptr);
  EXPECT_EQ(parse(R"({a: 1})"), nullptr);
  EXPECT_EQ(parse(R"({"a" 1})"), nullptr);
  EXPECT_EQ(parse(R"([1 2])"), nullptr);
  EXPECT_EQ(parse(R"({"a": "unclosed})"), nullptr);
  EXPECT_EQ(parse(R"({"a": tru})"), nullptr);
  EXPECT_EQ(parse(R"({"a": 1} trailing)"), nullptr);
  EXPECT_EQ(parse(R"({"a": 0x10})"), nullptr);
}

// A recursive descent walks the stack down with the document it is handed.
TEST_F(UTEST_JsonParser, refuses_a_document_nesting_away_the_stack)
{
  EXPECT_NE(parse(std::string(32U, '[') + std::string(32U, ']')), nullptr);
  EXPECT_EQ(parse(std::string(4096U, '[')), nullptr);
}

TEST_F(UTEST_JsonParser, reads_one_document_after_another)
{
  ASSERT_NE(parse(R"({"a": 1})"), nullptr);
  EXPECT_EQ(parse("{"), nullptr);
  EXPECT_NE(parse(R"([1])"), nullptr);
}
