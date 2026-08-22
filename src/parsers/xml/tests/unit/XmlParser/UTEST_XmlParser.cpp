#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "src/parsers/xml/XmlParser.h"

using namespace testing;
using iannotator::importers::parsers::XmlNode;
using iannotator::importers::parsers::XmlNodePtr;
using iannotator::importers::parsers::XmlParser;

namespace
{

class UTEST_XmlParser : public Test
{
 public:
  XmlParser parser;

  XmlNodePtr parse(const std::string& document)
  {
    return parser.parse(document);
  }
};

}  // namespace

// The descriptor the sibling exporters library writes for one image.
TEST_F(UTEST_XmlParser, reads_the_pascal_voc_descriptor)
{
  auto root = parse(R"(<?xml version="1.0" encoding="UTF-8"?>
<annotation>
  <folder>JPEGImages</folder>
  <filename>street.png</filename>
  <source>
    <database>The ImagesAnnotator annotations dataset</database>
  </source>
  <size>
    <width>640</width>
    <height>400</height>
    <depth>3</depth>
  </size>
  <segmented>0</segmented>
  <object>
    <name>dog</name>
    <pose>Unspecified</pose>
    <truncated>0</truncated>
    <difficult>0</difficult>
    <bndbox>
      <xmin>50</xmin>
      <ymin>20</ymin>
      <xmax>150</xmax>
      <ymax>60</ymax>
    </bndbox>
  </object>
  <object>
    <name>cat</name>
    <bndbox>
      <xmin>1</xmin>
      <ymin>2</ymin>
      <xmax>3</xmax>
      <ymax>4</ymax>
    </bndbox>
  </object>
</annotation>)");

  ASSERT_NE(root, nullptr);
  EXPECT_EQ(root->get_name(), "annotation");

  std::string fileName;

  ASSERT_TRUE(root->read_text("filename", fileName));
  EXPECT_EQ(fileName, "street.png");

  const XmlNode* size = root->find("size");

  ASSERT_NE(size, nullptr);

  double width{0.0};
  double height{0.0};

  ASSERT_TRUE(size->read_number("width", width));
  ASSERT_TRUE(size->read_number("height", height));
  EXPECT_DOUBLE_EQ(width, 640.0);
  EXPECT_DOUBLE_EQ(height, 400.0);

  std::size_t objects{0U};

  for (const auto& child : root->get_children()) {
    if (child->get_name() == "object") {
      ++objects;
    }
  }

  EXPECT_EQ(objects, 2U);

  const XmlNode* box = root->find("object")->find("bndbox");

  ASSERT_NE(box, nullptr);

  double xmax{0.0};

  ASSERT_TRUE(box->read_number("xmax", xmax));
  EXPECT_DOUBLE_EQ(xmax, 150.0);

  // The first element of that name is the one handed out, so the two objects
  // are told apart by the walk above and not by a lookup.
  std::string name;

  ASSERT_TRUE(root->find("object")->read_text("name", name));
  EXPECT_EQ(name, "dog");
}

TEST_F(UTEST_XmlParser, hands_out_no_element_of_another_name)
{
  auto root = parse("<annotation><filename>a.png</filename></annotation>");

  ASSERT_NE(root, nullptr);

  EXPECT_EQ(root->find("absent"), nullptr);

  std::string text;
  double value{0.0};

  EXPECT_FALSE(root->read_text("absent", text));
  EXPECT_FALSE(root->read_number("filename", value));
}

TEST_F(UTEST_XmlParser, reads_a_coordinate_written_with_a_fractional_part)
{
  auto root = parse("<bndbox><xmin> 50.5 </xmin></bndbox>");

  ASSERT_NE(root, nullptr);

  double xmin{0.0};

  ASSERT_TRUE(root->read_number("xmin", xmin));
  EXPECT_DOUBLE_EQ(xmin, 50.5);
}

TEST_F(UTEST_XmlParser, reads_the_prologue_and_the_comments_over)
{
  auto root = parse(R"(<?xml version="1.0"?>
<!-- written by a tool -->
<!DOCTYPE annotation SYSTEM "annotation.dtd">
<annotation><!-- and here as well --><filename>a.png</filename></annotation>
<!-- behind the root -->)");

  ASSERT_NE(root, nullptr);

  std::string fileName;

  ASSERT_TRUE(root->read_text("filename", fileName));
  EXPECT_EQ(fileName, "a.png");
}

TEST_F(UTEST_XmlParser, reads_an_internal_dtd_subset_over)
{
  auto root = parse(R"(<!DOCTYPE annotation [<!ELEMENT annotation (#PCDATA)>]>
<annotation><filename>a.png</filename></annotation>)");

  ASSERT_NE(root, nullptr);
  EXPECT_EQ(root->get_name(), "annotation");
}

// LabelImg marks its own descriptors with an attribute of the root element.
TEST_F(UTEST_XmlParser, reads_the_attributes_over)
{
  auto root =
      parse(R"(<annotation verified='yes' xmlns:x="http://a/b">)"
            R"(<object id="1"/><filename>a.png</filename></annotation>)");

  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->get_children().size(), 2U);
  EXPECT_EQ(root->get_children().front()->get_name(), "object");
  EXPECT_TRUE(root->get_children().front()->get_text().empty());
}

TEST_F(UTEST_XmlParser, resolves_the_entity_references)
{
  auto root = parse(
      "<annotation><name>a &amp; b &lt;c&gt; &quot;d&quot; &apos;e&apos; "
      "&#65;&#x42;</name></annotation>");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "a & b <c> \"d\" 'e' AB");
}

TEST_F(UTEST_XmlParser, resolves_a_character_reference_outside_ascii_as_utf8)
{
  auto root = parse("<annotation><name>&#228;</name></annotation>");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "\xC3\xA4");
}

// The entity may well be declared in a document type declaration this reader
// reads over, so it costs the descriptor no import.
TEST_F(UTEST_XmlParser, leaves_an_unknown_entity_reference_in_the_text)
{
  auto root = parse("<annotation><name>a&nbsp;b &amp c</name></annotation>");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "a&nbsp;b &amp c");
}

TEST_F(UTEST_XmlParser, reads_a_cdata_section_as_the_text_it_holds)
{
  auto root =
      parse("<annotation><name><![CDATA[a & <b>]]>c</name></annotation>");

  ASSERT_NE(root, nullptr);

  std::string name;

  ASSERT_TRUE(root->read_text("name", name));
  EXPECT_EQ(name, "a & <b>c");
}

TEST_F(UTEST_XmlParser, refuses_a_malformed_document)
{
  EXPECT_EQ(parse(""), nullptr);
  EXPECT_EQ(parse("<annotation>"), nullptr);
  EXPECT_EQ(parse("<annotation></size>"), nullptr);
  EXPECT_EQ(parse("<annotation></annotation><size></size>"), nullptr);
  EXPECT_EQ(parse("<annotation><size></annotation>"), nullptr);
  EXPECT_EQ(parse("<>"), nullptr);
  EXPECT_EQ(parse("<annotation x></annotation>"), nullptr);
  EXPECT_EQ(parse("<annotation x=y></annotation>"), nullptr);
  EXPECT_EQ(parse("<annotation><!-- unclosed --></annotation"), nullptr);
  EXPECT_EQ(parse("no markup at all"), nullptr);
}

// A recursive descent walks the stack down with the document it is handed.
TEST_F(UTEST_XmlParser, refuses_a_document_nesting_away_the_stack)
{
  std::string deep;
  std::string closing;

  for (std::size_t iter = 0U; iter < 32U; ++iter) {
    deep += "<a>";
    closing += "</a>";
  }

  EXPECT_NE(parse(deep + closing), nullptr);

  for (std::size_t iter = 0U; iter < 4096U; ++iter) {
    deep += "<a>";
  }

  EXPECT_EQ(parse(deep), nullptr);
}

TEST_F(UTEST_XmlParser, reads_one_document_after_another)
{
  ASSERT_NE(parse("<annotation><a>1</a></annotation>"), nullptr);
  EXPECT_EQ(parse("<annotation>"), nullptr);
  EXPECT_NE(parse("<annotation/>"), nullptr);
}
