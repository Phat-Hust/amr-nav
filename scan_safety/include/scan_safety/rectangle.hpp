#ifndef SCAN_SAFETY__RECTANGLE_HPP_
#define SCAN_SAFETY__RECTANGLE_HPP_

#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace scan_safety
{
struct Point
{
  double x;
  double y;
};

class Rectangle
{
public:
  // Points must follow the perimeter, clockwise or counterclockwise.
  explicit Rectangle(const std::vector<double> & coordinates)
  {
    if (coordinates.size() != 8) {
      throw std::invalid_argument("A rectangle needs four [x, y] points (8 doubles)");
    }
    for (std::size_t i = 0; i < points_.size(); ++i) {
      const double x = coordinates[2 * i];
      const double y = coordinates[2 * i + 1];
      if (!std::isfinite(x) || !std::isfinite(y)) {
        throw std::invalid_argument("Rectangle coordinates must be finite");
      }
      points_[i] = {x, y};
    }
    double winding = 0.0;
    for (std::size_t i = 0; i < points_.size(); ++i) {
      const auto a = edge(i);
      const auto b = edge((i + 1) % points_.size());
      const double a_length = std::hypot(a.x, a.y);
      const double b_length = std::hypot(b.x, b.y);
      const double cross = a.x * b.y - a.y * b.x;
      if (a_length < 1e-6 || b_length < 1e-6 ||
        std::abs(a.x * b.x + a.y * b.y) > 1e-6 * a_length * b_length ||
        (i > 0 && cross * winding <= 0.0))
      {
        throw std::invalid_argument("Points must form a nondegenerate rectangle in perimeter order");
      }
      winding = cross;
    }
  }

  bool contains(double x, double y) const
  {
    if (!std::isfinite(x) || !std::isfinite(y)) {
      return false;
    }
    // Projection onto the two perpendicular sides also supports rotated rectangles.
    const auto a = edge(0);
    const Point b{points_[3].x - points_[0].x, points_[3].y - points_[0].y};
    const Point offset{x - points_[0].x, y - points_[0].y};
    const double u = (offset.x * a.x + offset.y * a.y) / (a.x * a.x + a.y * a.y);
    const double v = (offset.x * b.x + offset.y * b.y) / (b.x * b.x + b.y * b.y);
    constexpr double tolerance = 1e-9;
    return u >= -tolerance && u <= 1.0 + tolerance &&
           v >= -tolerance && v <= 1.0 + tolerance;
  }

  bool contains(const Rectangle & other) const
  {
    for (const auto & point : other.points_) {
      if (!contains(point.x, point.y)) {
        return false;
      }
    }
    return true;
  }

private:
  Point edge(std::size_t i) const
  {
    const auto & a = points_[i];
    const auto & b = points_[(i + 1) % points_.size()];
    return {b.x - a.x, b.y - a.y};
  }

  std::array<Point, 4> points_;
};
}  // namespace scan_safety

#endif  // SCAN_SAFETY__RECTANGLE_HPP_
