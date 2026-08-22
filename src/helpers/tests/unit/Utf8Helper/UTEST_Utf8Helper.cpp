#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "src/helpers/Utf8Helper.h"

using namespace testing;
using iannotator::importers::helpers::Utf8Helper;

namespace
{

std::string written(const unsigned long& codepoint)
{
  std::string value;

  Utf8Helper::append(codepoint, value);

  return value;
}

}  // namespace

// The four lengths of the encoding, each at the first code point which needs
// it: the JSON \uXXXX escape and the XML &#NN; reference both arrive here.
TEST(UTEST_Utf8Helper, append_writes_every_code_point_length)
{
  EXPECT_EQ(written(0x41UL), "A");
  EXPECT_EQ(written(0xE4UL), "\xC3\xA4");
  EXPECT_EQ(written(0x500UL), "\xD4\x80");
  EXPECT_EQ(written(0x1F600UL), "\xF0\x9F\x98\x80");
}

TEST(UTEST_Utf8Helper, append_writes_onto_the_end_of_the_given_text)
{
  std::string value{"a"};

  Utf8Helper::append(0x42UL, value);
  Utf8Helper::append(0xE4UL, value);

  EXPECT_EQ(value, "aB\xC3\xA4");
}
