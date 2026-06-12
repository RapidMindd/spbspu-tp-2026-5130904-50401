#include <ios>
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

  struct del
  {
    std::string s;
  };

  struct label
  {
    std::string s;
  };

  struct ll
  {
    long long& num;
  };

  struct symb
  {
    char& c;
  };

  struct str
  {
    std::string& s;
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
    out << "key1 " << data.key1 << "ll";
    out << ':';
    out << "key2 " << '\'' << data.key2 << '\'';
    out << ':';
    out << "key3 " << '\"' << data.key3 << '\"';
    out << ":)";
    return out;
  }

  std::istream& operator>>(std::istream& in, del&& expected)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    for (size_t i = 0; i < expected.s.size(); ++i)
    {
      char c = 0;
      in.get(c);
      if (!in || c != expected.s[i])
      {
        in.setstate(std::ios_base::failbit);
        break;
      }
    }
    return in;
  }

  std::istream& operator>>(std::istream& in, str&& dest)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    return std::getline(in >> del{"\""}, dest.s, '\"');
  }

  std::istream& operator>>(std::istream& in, symb&& dest)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    return in >> del{"\'"} >> dest.c >> del{"\'"};
  }

  std::istream& operator>>(std::istream& in, ll&& dest)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    return in >> dest.num >> del{"ll"};
  }

  std::istream& operator>>(std::istream& in, label&& expected)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    return in >> del{expected.s};
  }

  std::istream& operator>>(std::istream& in, DataStruct& data)
  {
    std::istream::sentry s(in);
    if (!s)
    {
      return in;
    }
    IOguard guard(in);
    DataStruct input;
    {
      in >> std::skipws;
      in >> del{"(:"};
      in >> label{"key1 "} >> ll{input.key1} >> del{":"};
      in >> label{"key2 "} >> symb{input.key2} >> del{":"};
      in >> label{"key3 "} >> str{input.key3};
      in >> del{":)"};
    }
    if (in)
    {
      data = input;
    }
    return in;
  }
}

int main()
{
  tarasenko::DataStruct data;
  std::cin >> data;
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
