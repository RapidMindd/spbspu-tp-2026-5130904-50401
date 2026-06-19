#include <iostream>
#include <istream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>

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
}

int main()
{
  // if (argc != 2)
  // {
  //   std::cerr << "1 argument expected\n";
  //   return 1;
  // }

  using namespace tarasenko;
  Polygon polygon;
  std::cin >> polygon;
  if (!std::cin)
  {
    return 1;
  }
  std::cout << polygon.points[0].x << ' ' << polygon.points[0].y << '\n';
  std::cout << polygon.points[1].x << ' ' << polygon.points[1].y << '\n';
  return 0;
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
