#include <iostream>
#include <istream>
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

  void calculateAreas(std::istream&, std::ostream& out, const Polygons& polygons)
  {
    IOguard guard(out);
    std::vector< double > areas;
    std::transform(polygons.begin(), polygons.end(), std::back_inserter(areas), getPolygonArea);
    out << std::fixed << std::setprecision(1) << std::accumulate(areas.begin(), areas.end(), 0.0) << '\n';
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
