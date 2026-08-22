#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "src/helpers/StringHelper.h"

using namespace testing;
using iannotator::importers::helpers::StringHelper;

// The whitespace a descriptor entry is laid out with is no part of its value:
// the obj.data entries carry it around their equals sign, an XML element may
// hold its value on a line of its own and a YAML entry is indented.
TEST(UTEST_StringHelper, trim_drops_the_whitespace_around_the_value)
{
  EXPECT_EQ(StringHelper::trim("  data/obj.names  "), "data/obj.names");
  EXPECT_EQ(StringHelper::trim("\t\r\n dog \n\r\t"), "dog");
  EXPECT_EQ(StringHelper::trim("dog"), "dog");
}

// A value made of nothing but whitespace names nothing at all.
TEST(UTEST_StringHelper, trim_gives_an_empty_value_back_empty)
{
  EXPECT_TRUE(StringHelper::trim("").empty());
  EXPECT_TRUE(StringHelper::trim(" \t\r\n").empty());
}

// Only the ends are trimmed: an annotation name may hold a blank of its own.
TEST(UTEST_StringHelper, trim_keeps_the_whitespace_inside_the_value)
{
  EXPECT_EQ(StringHelper::trim("  a cat and a dog  "), "a cat and a dog");
}
