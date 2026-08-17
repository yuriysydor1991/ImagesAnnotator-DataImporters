#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/helpers/TypeHelper.h"

using namespace testing;
using iannotator::importers::helpers::TypeHelper;

TEST(UTEST_TypeHelper, toD_casts_the_value_to_double)
{
  EXPECT_DOUBLE_EQ(TypeHelper::toD(5), 5.0);
  EXPECT_DOUBLE_EQ(TypeHelper::toD(-3), -3.0);
  EXPECT_DOUBLE_EQ(TypeHelper::toD(2.5f), 2.5);
}
