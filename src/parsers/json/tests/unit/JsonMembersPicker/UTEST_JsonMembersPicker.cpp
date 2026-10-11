#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "src/parsers/json/JsonMembersPicker.h"

using namespace testing;
using iannotator::importers::parsers::JsonMembersPicker;

namespace
{

using Path = JsonMembersPicker::Path;
using Picked = std::map<Path, std::string>;

/**
 * @brief Hands the document out in the pieces of the given size, the way a
 * decompressing stream does, and counts how much of it was asked for.
 */
class PiecesSource
{
 public:
  PiecesSource(std::string ndocument, std::size_t npiece)
      : document{std::move(ndocument)}, piece{npiece}
  {
  }

  std::size_t operator()(char* buffer, std::size_t size)
  {
    const std::size_t count =
        std::min({size, piece, document.size() - consumed});

    std::memcpy(buffer, document.data() + consumed, count);

    consumed += count;

    return count;
  }

  std::string document;
  std::size_t piece;
  std::size_t consumed{0U};
};

class UTEST_JsonMembersPicker : public Test
{
 public:
  /// @brief Picks the wanted members of the document handed out in the
  /// pieces of the size, the receiver asking to go on for all of them.
  static bool pick(const std::string& document, const std::vector<Path>& wanted,
                   Picked& picked, std::size_t piece = 3U,
                   std::size_t* consumed = nullptr)
  {
    PiecesSource source{document, piece};

    JsonMembersPicker picker{[&source](char* buffer, std::size_t size) {
      return source(buffer, size);
    }};

    const bool read = picker.pick(
        wanted, [&picked](const Path& path, const std::string& text) {
          picked[path] = text;
          return true;
        });

    if (consumed != nullptr) {
      *consumed = source.consumed;
    }

    return read;
  }
};

/// @brief The shape of a dxxwjz2p project of the weights after the annotations
const std::string project = R"({
  "format": "dxxwjz2p",
  "version": 1,
  "generator": "darknetxx 0.11.0",
  "network": {"cfg": "[net]\nbatch=64\n{[\"", "classes": [
    {"index": 0, "name": "cat"},
    {"index": 1, "name": "dog"}
  ]},
  "training": {"seconds": 3725, "iteration": [1,2,3], "loss": [812.5,790.25,701]},
  "annotations": {"ImagesAnnotator": {"version": "0.12.0"}, "annotations": [
    {"db": {"path": "images"}, "annotations": [
      {"rel_path": "park.jpg", "width": 640, "height": 480, "annotations": [{"name": "dog", "rect": {"x": 200, "y": 130, "width": 48, "height": 52}}]}
    ]}
  ]},
  "header": {"major": 0, "minor": 2, "revision": 5, "seen": 192},
  "layers": [
    {"index": 0, "type": "convolutional", "arrays": {"biases": [0.5,-0.25,"0x7fc00000"]}}
  ]
})";

}  // namespace

TEST_F(UTEST_JsonMembersPicker, picks_the_text_of_the_wanted_values)
{
  Picked picked;

  ASSERT_TRUE(
      pick(project, {{"format"}, {"version"}, {"network", "classes"}}, picked));

  ASSERT_EQ(picked.size(), 3U);
  EXPECT_EQ(picked[Path{"format"}], R"("dxxwjz2p")");
  EXPECT_EQ(picked[Path{"version"}], "1");
  EXPECT_EQ(picked[(Path{"network", "classes"})], R"([
    {"index": 0, "name": "cat"},
    {"index": 1, "name": "dog"}
  ])");
}

// The brackets and the quotes inside of the strings close nothing, whatever
// the piece they arrive in.
TEST_F(UTEST_JsonMembersPicker, steps_over_the_brackets_of_the_strings)
{
  for (std::size_t piece : {1U, 2U, 7U, 4096U}) {
    Picked picked;

    ASSERT_TRUE(pick(project, {{"annotations"}}, picked, piece)) << piece;

    ASSERT_EQ(picked.size(), 1U) << piece;
    EXPECT_EQ(picked.begin()->second.front(), '{') << piece;
    EXPECT_EQ(picked.begin()->second.back(), '}') << piece;
    EXPECT_NE(picked.begin()->second.find("park.jpg"), std::string::npos);
  }
}

// The weights of a trained project come after its annotations: once every
// wanted value is handed over nothing more of the document is asked for.
TEST_F(UTEST_JsonMembersPicker, stops_reading_after_the_last_wanted_value)
{
  Picked picked;
  std::size_t consumed{0U};

  ASSERT_TRUE(
      pick(project, {{"format"}, {"annotations"}}, picked, 16U, &consumed));

  EXPECT_EQ(picked.size(), 2U);
  EXPECT_LT(consumed, project.find("\"layers\""));
}

TEST_F(UTEST_JsonMembersPicker, leaves_out_the_members_the_document_lacks)
{
  Picked picked;

  ASSERT_TRUE(
      pick(project, {{"format"}, {"absent"}, {"network", "absent"}}, picked));

  ASSERT_EQ(picked.size(), 1U);
  EXPECT_EQ(picked.begin()->first, Path{"format"});
}

// A path through a value which is no object names nothing.
TEST_F(UTEST_JsonMembersPicker, reads_no_member_of_a_value_of_no_object)
{
  Picked picked;

  ASSERT_TRUE(
      pick(R"({"a": [{"b": 1}], "c": "d"})", {{"a", "b"}, {"c", "d"}}, picked));

  EXPECT_TRUE(picked.empty());
}

TEST_F(UTEST_JsonMembersPicker, picks_the_first_member_of_a_repeated_key)
{
  Picked picked;

  ASSERT_TRUE(pick(R"({"a": 1, "a": 2, "b": 3})", {{"a"}, {"b"}}, picked));

  EXPECT_EQ(picked[Path{"a"}], "1");
  EXPECT_EQ(picked[Path{"b"}], "3");
}

TEST_F(UTEST_JsonMembersPicker, resolves_the_escapes_of_the_keys)
{
  Picked picked;

  ASSERT_TRUE(
      pick(R"({"format": "x", "a\"b": true})", {{"format"}, {"a\"b"}}, picked));

  EXPECT_EQ(picked[Path{"format"}], R"("x")");
  EXPECT_EQ(picked[Path{"a\"b"}], "true");
}

TEST_F(UTEST_JsonMembersPicker, the_receiver_stops_the_reading)
{
  PiecesSource source{project, 5U};

  JsonMembersPicker picker{[&source](char* buffer, std::size_t size) {
    return source(buffer, size);
  }};

  std::vector<Path> received;

  EXPECT_TRUE(picker.pick({{"format"}, {"annotations"}},
                          [&received](const Path& path, const std::string&) {
                            received.push_back(path);
                            return false;
                          }));

  ASSERT_EQ(received.size(), 1U);
  EXPECT_EQ(received.front(), Path{"format"});
  EXPECT_LT(source.consumed, project.find("\"annotations\""));
}

TEST_F(UTEST_JsonMembersPicker, refuses_the_malformed_documents)
{
  for (const std::string& document :
       {std::string{}, std::string{"[]"}, std::string{"{"},
        std::string{R"({"a":)"}, std::string{R"({"a" 1})"},
        std::string{R"({"a": [1, 2})"}, std::string{R"({"a": "x)"},
        std::string{R"({"a": 1} x)"}, std::string{"{a: 1}"},
        std::string{R"({"a": {"b": ]}})"}, std::string{R"({"a": 1,})"}}) {
    Picked picked;

    EXPECT_FALSE(pick(document, {{"z"}}, picked)) << document;
  }
}

TEST_F(UTEST_JsonMembersPicker, refuses_the_values_nested_too_deep)
{
  const std::string deep =
      R"({"a": )" + std::string(100U, '[') + std::string(100U, ']') + "}";

  Picked picked;

  EXPECT_FALSE(pick(deep, {{"z"}}, picked));
}

TEST_F(UTEST_JsonMembersPicker, nothing_wanted_reads_nothing)
{
  Picked picked;
  std::size_t consumed{0U};

  EXPECT_TRUE(pick(project, {}, picked, 3U, &consumed));
  EXPECT_EQ(consumed, 0U);
}
