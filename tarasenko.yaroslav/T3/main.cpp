#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <fstream>
#include <unordered_map>
#include <numeric>
#include <limits>
#include <iomanip>
#include <sstream>

namespace tarasenko
{
  class IOguard
  {
  public:
    explicit IOguard(std::basic_ios< char >& s);
    ~IOguard();
  private:
    std::basic_ios< char >& s_;
    std::streamsize width_;
    char fill_;
    std::streamsize precision_;
    std::basic_ios< char >::fmtflags fmt_;
  };

  struct Point
  {
    int x;
    int y;
  };

  struct Polygon
  {
    std::vector< Point > points;
  };

  struct Triangle
  {
    Point points[3];
  };

  using Polygons = std::vector< Polygon >;

  struct del
  {
    char c;
  };

  std::istream& operator>>(std::istream& in, del&& expected)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    char c = '0';
    in >> c;
    if (in && (c != expected.c))
    {
      in.setstate(std::ios::failbit);
    }
    return in;
  }

  std::istream& operator>>(std::istream& in, Point& point)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    return in >> del{'('} >> point.x >> del{';'} >> point.y >> del{')'};
  }

  std::istream& operator>>(std::istream& in, Polygon& polygon)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    int count = 0;
    in >> count;
    if (count < 3)
    {
      in.setstate(std::ios::failbit);
      return in;
    }
    std::vector< Point > points;
    using iit = std::istream_iterator< Point >;
    std::copy_n(iit(in), count, std::back_inserter(points));
    if (in)
    {
      polygon.points = points;
    }
    return in;
  }

  std::ostream& operator<<(std::ostream& out, const Point& point)
  {
    std::ostream::sentry s(out);
    if (!s)
    {
      return out;
    }
    IOguard guard(out);
    out << '(' << point.x << ';' << point.y << ')';
    return out;
  }

  std::ostream& operator<<(std::ostream& out, const Polygon& polygon)
  {
    std::ostream::sentry s(out);
    if (!s)
    {
      return out;
    }
    IOguard guard(out);
    out << polygon.points.size() << ' ';
    using oit = std::ostream_iterator< Point >;
    std::copy(polygon.points.begin(), polygon.points.end(), oit(out, " "));
    return out;
  }

  struct TriangleGenerator
  {
    const Polygon& polygon;
    int index;

    TriangleGenerator(const Polygon& polygon):
      polygon(polygon),
      index(0)
    {}

    Triangle operator()()
    {
      Triangle triangle;
      triangle.points[0] = polygon.points[0];
      triangle.points[1] = polygon.points[index + 1];
      triangle.points[2] = polygon.points[index + 2];
      ++index;
      return triangle;
    }
  };

  double getTriangleArea(const Triangle& triangle)
  {
    Point a = triangle.points[0];
    Point b = triangle.points[1];
    Point c = triangle.points[2];
    return 0.5 * std::abs(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
  }

  double getPolygonArea(const Polygon& polygon)
  {
    std::vector< Triangle > triangles;
    size_t triangle_count = polygon.points.size() - 2;
    std::generate_n(std::back_inserter(triangles), triangle_count, TriangleGenerator(polygon));
    std::vector< double > areas;
    std::transform(triangles.begin(), triangles.end(), std::back_inserter(areas), getTriangleArea);
    return std::accumulate(areas.begin(), areas.end(), 0.0);
  }

  double getAreaIfRightParity(const Polygon& polygon, int remainder)
  {
    if (polygon.points.size() % 2 == remainder)
    {
      return getPolygonArea(polygon);
    }
    return 0.0;
  }

  double getAreaIfRightCount(const Polygon& polygon, size_t count)
  {
    if (polygon.points.size() == count)
    {
      return getPolygonArea(polygon);
    }
    return 0.0;
  }

  void calculateAreas(std::istream& in, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    out << std::fixed << std::setprecision(1);
    std::string argument;
    in >> argument;
    int remainder = 0;
    if (argument == "MEAN")
    {
      if (polygons.size() == 0)
      {
        throw std::logic_error("Not enough polygons");
      }
      std::vector< double > areas;
      std::transform(polygons.begin(), polygons.end(), std::back_inserter(areas), getPolygonArea);
      out << std::accumulate(areas.begin(), areas.end(), 0.0) / polygons.size() << '\n';
      return;
    }
    else if (argument == "EVEN")
    {
      remainder = 0;
    }
    else if (argument == "ODD")
    {
      remainder = 1;
    }
    else
    {
      std::istringstream stream(argument);
      size_t vertexes = 0;
      if (stream >> vertexes && stream.eof() && vertexes > 2)
      {
        std::vector< double > areas;
        using namespace std::placeholders;
        std::transform(polygons.begin(), polygons.end(),
          std::back_inserter(areas), std::bind(getAreaIfRightCount, _1, vertexes));
        out << std::accumulate(areas.begin(), areas.end(), 0.0) << '\n';
        return;
      }
      else
      {
        throw std::invalid_argument("Unknown argument");
      }
    }
    std::vector< double > areas;
    using namespace std::placeholders;
    std::transform(polygons.begin(), polygons.end(),
      std::back_inserter(areas), std::bind(getAreaIfRightParity, _1, remainder));
    out << std::accumulate(areas.begin(), areas.end(), 0.0) << '\n';
  }

  size_t getVertexes(const Polygon& polygon)
  {
    return polygon.points.size();
  }

  void calculateMaximums(std::istream& in, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    out << std::fixed << std::setprecision(1);
    std::string argument;
    in >> argument;
    if (polygons.size() == 0)
    {
      throw std::logic_error("Not enough polygons");
    }
    if (argument == "AREA")
    {
      std::vector< double > areas;
      std::transform(polygons.begin(), polygons.end(), std::back_inserter(areas), getPolygonArea);
      out << *std::max_element(areas.begin(), areas.end()) << '\n';
    }
    else if (argument == "VERTEXES")
    {
      std::vector< size_t > vertexes;
      std::transform(polygons.begin(), polygons.end(), std::back_inserter(vertexes), getVertexes);
      out << *std::max_element(vertexes.begin(), vertexes.end()) << '\n';
    }
    else
    {
      throw std::invalid_argument("Unknown argument");
    }
  }

  void calculateMinimums(std::istream& in, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    out << std::fixed << std::setprecision(1);
    std::string argument;
    in >> argument;
    if (polygons.size() == 0)
    {
      throw std::logic_error("Not enough polygons");
    }
    if (argument == "AREA")
    {
      std::vector< double > areas;
      std::transform(polygons.begin(), polygons.end(), std::back_inserter(areas), getPolygonArea);
      out << *std::min_element(areas.begin(), areas.end()) << '\n';
    }
    else if (argument == "VERTEXES")
    {
      std::vector< size_t > vertexes;
      std::transform(polygons.begin(), polygons.end(), std::back_inserter(vertexes), getVertexes);
      out << *std::min_element(vertexes.begin(), vertexes.end()) << '\n';
    }
    else
    {
      throw std::invalid_argument("Unknown argument");
    }
  }

  bool isEvenVertexes(const Polygon& polygon)
  {
    return polygon.points.size() % 2 == 0;
  }

  bool isOddVertexes(const Polygon& polygon)
  {
    return polygon.points.size() % 2 == 1;
  }

  bool isRightCountVertexes(const Polygon& polygon, size_t vertexes)
  {
    return polygon.points.size() == vertexes;
  }

  void countPolygons(std::istream& in, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    out << std::fixed << std::setprecision(1);
    std::string argument;
    in >> argument;
    if (argument == "EVEN")
    {
      out << std::count_if(polygons.begin(), polygons.end(), isEvenVertexes) << '\n';
    }
    else if (argument == "ODD")
    {
      out << std::count_if(polygons.begin(), polygons.end(), isOddVertexes) << '\n';
    }
    else
    {
      std::istringstream stream(argument);
      size_t vertexes = 0;
      if (stream >> vertexes && stream.eof() && vertexes > 2)
      {
        using namespace std::placeholders;
        out << std::count_if(polygons.begin(), polygons.end(), std::bind(isRightCountVertexes, _1, vertexes)) << '\n';
      }
      else
      {
        throw std::invalid_argument("Unknown argument");
      }
    }
  }

  struct Segment
  {
    Point a;
    Point b;
  };

  long long getOrientedTriangleArea(const Point& a, const Point& b, const Point& c)
  {
    return a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
  }

  bool isCoordinateBetween(int left, int value, int right)
  {
    return std::min(left, right) <= value && value <= std::max(left, right);
  }

  bool isPointOnSegment(const Point& point, const Segment& segment)
  {
    bool ans = getOrientedTriangleArea(segment.a, segment.b, point) == 0;
    ans = ans && isCoordinateBetween(segment.a.x, point.x, segment.b.x);
    ans = ans && isCoordinateBetween(segment.a.y, point.y, segment.b.y);
    return ans;
  }

  bool areSegmentsIntersected(const Segment& lhs, const Segment& rhs)
  {
    long long lhs_first_area = getOrientedTriangleArea(lhs.a, lhs.b, rhs.a);
    long long lhs_second_area = getOrientedTriangleArea(lhs.a, lhs.b, rhs.b);
    long long rhs_first_area = getOrientedTriangleArea(rhs.a, rhs.b, lhs.a);
    long long rhs_second_area = getOrientedTriangleArea(rhs.a, rhs.b, lhs.b);

    if (lhs_first_area == 0 && isPointOnSegment(rhs.a, lhs))
    {
      return true;
    }
    if (lhs_second_area == 0 && isPointOnSegment(rhs.b, lhs))
    {
      return true;
    }
    if (rhs_first_area == 0 && isPointOnSegment(lhs.a, rhs))
    {
      return true;
    }
    if (rhs_second_area == 0 && isPointOnSegment(lhs.b, rhs))
    {
      return true;
    }
    return (lhs_first_area > 0) != (lhs_second_area > 0) && (rhs_first_area > 0) != (rhs_second_area > 0);
  }

  Segment getPolygonEdge(const Polygon& polygon, size_t index)
  {
    if (index == polygon.points.size() - 1)
    {
      return Segment{polygon.points[index], polygon.points[0]};
    }
    return Segment{polygon.points[index], polygon.points[index + 1]};
  }

  bool areEdgeAndPolygonIntersected(const Segment& edge, const Polygon& polygon)
  {
    std::vector< size_t > indexes(polygon.points.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    std::vector< Segment > edges;
    using namespace std::placeholders;
    std::transform(indexes.begin(), indexes.end(), std::back_inserter(edges),
      std::bind(getPolygonEdge, std::cref(polygon), _1));
    return std::any_of(edges.begin(), edges.end(), std::bind(areSegmentsIntersected, edge, _1));
  }

  bool isIntersected(const Polygon& lhs, const Polygon& rhs)
  {
    std::vector< size_t > indexes(lhs.points.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    std::vector< Segment > edges;
    using namespace std::placeholders;
    std::transform(indexes.begin(), indexes.end(), std::back_inserter(edges),
      std::bind(getPolygonEdge, std::cref(lhs), _1));
    return std::any_of(edges.begin(), edges.end(), std::bind(areEdgeAndPolygonIntersected, _1, std::cref(rhs)));
  }

  void countIntersections(std::istream& in, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    Polygon current;
    in >> current;
    if (!in)
    {
      throw std::invalid_argument("Incorrect polygon");
    }
    using namespace std::placeholders;
    out << std::count_if(polygons.begin(), polygons.end(), std::bind(isIntersected, _1, current)) << '\n';
  }

  bool isRightAngle(const Point& prev, const Point& target, const Point& next)
  {
    int ba_x = prev.x - target.x;
    int ba_y = prev.y - target.y;
    int bc_x = next.x - target.x;
    int bc_y = next.y - target.y;
    return ba_x * bc_x + ba_y * bc_y == 0;
  }

  bool isRightAngleInPolygon(const Polygon& polygon, size_t index)
  {
    size_t prev = index == 0 ? polygon.points.size() - 1 : index - 1;
    size_t next = index == polygon.points.size() - 1 ? 0 : index + 1;
    return isRightAngle(polygon.points[prev], polygon.points[index], polygon.points[next]);
  }

  bool hasRightAngle(const Polygon& polygon)
  {
    std::vector< size_t > indexes(polygon.points.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    using namespace std::placeholders;
    return std::any_of(indexes.begin(), indexes.end(), std::bind(isRightAngleInPolygon, std::cref(polygon), _1));
  }

  void countRightShapes(std::istream&, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    out << std::count_if(polygons.begin(), polygons.end(), hasRightAngle) << '\n';
  }
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "1 argument expected\n";
    return 1;
  }

  using namespace tarasenko;
  std::vector< Polygon > polygons;
  using iit = std::istream_iterator< Polygon >;
  std::ifstream in(argv[1]);
  std::copy(iit(in), iit{}, std::back_inserter(polygons));

  using cmd_t = std::function< void(std::istream&, std::ostream&, Polygons&) >;
  using const_cmd_t = std::function< void(std::istream&, std::ostream&, const Polygons&) >;
  std::unordered_map< std::string, cmd_t > cmds;

  std::unordered_map< std::string, const_cmd_t > const_cmds;
  cmds["AREA"] = calculateAreas;
  cmds["MAX"] = calculateMaximums;
  cmds["MIN"] = calculateMinimums;
  cmds["COUNT"] = countPolygons;
  cmds["INTERSECTIONS"] = countIntersections;
  cmds["RIGHTSHAPES"] = countRightShapes;

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      cmds.at(cmd)(std::cin, std::cout, polygons);
    }
    catch (const std::exception&)
    {
      std::cout << "<INVALID COMMAND>\n";
      auto toignore = std::numeric_limits< std::streamsize >::max();
      std::cin.clear();
      std::cin.ignore(toignore, '\n');
    }
  }
}

tarasenko::IOguard::IOguard(std::basic_ios< char >& s):
  s_(s),
  width_(s.width()),
  fill_(s.fill()),
  precision_(s.precision()),
  fmt_(s.flags())
{}

tarasenko::IOguard::~IOguard()
{
  s_.width(width_);
  s_.fill(fill_);
  s_.precision(precision_);
  s_.flags(fmt_);
}
