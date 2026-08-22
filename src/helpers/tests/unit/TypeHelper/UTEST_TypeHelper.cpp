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

// The rounding every layout writing a fraction is read back through.
TEST(UTEST_TypeHelper, toPixels_rounds_onto_the_nearest_pixel)
{
  EXPECT_EQ(TypeHelper::toPixels(5.0), 5);
  EXPECT_EQ(TypeHelper::toPixels(5.4), 5);
  EXPECT_EQ(TypeHelper::toPixels(5.5), 6);
  EXPECT_EQ(TypeHelper::toPixels(-2.5), -3);
}
