#include <limits>
#include <vector>
#include <gtest/gtest.h>
#include "scan_safety/rectangle.hpp"

using scan_safety::Rectangle;

TEST(Rectangle, IncludesInteriorAndBoundary)
{
  const Rectangle rectangle({-1., -0.5, 1., -0.5, 1., 0.5, -1., 0.5});
  EXPECT_TRUE(rectangle.contains(0., 0.));
  EXPECT_TRUE(rectangle.contains(-1., 0.5));
  EXPECT_TRUE(rectangle.contains(1., 0.));
  EXPECT_FALSE(rectangle.contains(1.01, 0.));
  EXPECT_FALSE(rectangle.contains(0., -0.51));
  EXPECT_FALSE(rectangle.contains(std::numeric_limits<double>::quiet_NaN(), 0.));
}

TEST(Rectangle, SupportsRotatedAndClockwisePoints)
{
  const Rectangle rotated({0., 1., 1., 0., 0., -1., -1., 0.});
  EXPECT_TRUE(rotated.contains(0., 0.));
  EXPECT_TRUE(rotated.contains(0.5, 0.5));
  EXPECT_FALSE(rotated.contains(0.9, 0.9));
}

TEST(Rectangle, RejectsMalformedAndNonrectangularZones)
{
  EXPECT_THROW(Rectangle(std::vector<double>{0., 0.}), std::invalid_argument);
  EXPECT_THROW(Rectangle({0., 0., 0., 0., 1., 1., 0., 1.}), std::invalid_argument);
  EXPECT_THROW(Rectangle({0., 0., 1., 0., 2., 1., 0., 1.}), std::invalid_argument);
  EXPECT_THROW(Rectangle({0., 0., 1., 1., 1., 0., 0., 1.}), std::invalid_argument);
  EXPECT_THROW(Rectangle({0., 0., 1., 0., 1., 1., 0.,
    std::numeric_limits<double>::infinity()}), std::invalid_argument);
}

TEST(Rectangle, ChecksNestedZones)
{
  const Rectangle outer({-1., -1., 1., -1., 1., 1., -1., 1.});
  const Rectangle inner({-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5});
  EXPECT_TRUE(outer.contains(inner));
  EXPECT_FALSE(inner.contains(outer));
}
