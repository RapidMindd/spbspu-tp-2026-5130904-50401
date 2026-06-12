#include <iostream>
#include <ostream>
#include <string>

namespace tarasenko
{
  struct DataStruct
  {
    long long key1;
    char key2;
    std::string key3;
  };

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

  std::ostream& operator<<(std::ostream& out, const DataStruct& data)
  {
    std::ostream::sentry s(out);
    if (!s)
    {
      return out;
    }
    IOguard guard(out);
    out << "(:";
    out << "key1 " << data.key1 << "LL";
    out << ':';
    out << "key2 " << '\'' << data.key2 << '\'';
    out << ':';
    out << "key3 " << '\"' << data.key3 << '\"';
    out << ":)";
    return out;
  }
}

int main()
{
  tarasenko::DataStruct data;
  data.key1 = 1;
  data.key2 = 'a';
  data.key3 = "Hello";
  std::cout << data << '\n';
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
