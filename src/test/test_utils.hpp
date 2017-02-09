// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/config.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/platform.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/utils.hpp"

#include <catch/catch.hpp>

#if defined(FSPP_IS_WIN)
#include <WinError.h>
#endif

#include <algorithm>
#include <iostream>
#include <ostream>
#include <random>
#include <set>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>


namespace eyestep {
namespace filesystem {

inline std::ostream&
operator<<(std::ostream& os, file_type ty)
{
  os << "#" << static_cast<int>(ty);
  return os;
}


inline std::ostream&
operator<<(std::ostream& os, perms p)
{
  os << "<perms: " << static_cast<uint32_t>(p) << ">";
  return os;
}


namespace tests {

inline std::string
make_random_string(size_t n)
{
  static const auto letters = std::string("abcdefghijklmnopqrstuvwxzy0123456789");

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, static_cast<int>(letters.size()) - 1);

  std::string result(n, 0);
  std::generate_n(
    result.begin(), n, [&]() { return letters[static_cast<size_t>(dis(gen))]; });
  return result;
}


inline std::string
read_file(const path& p)
{
  std::string data;
  with_stream_for_reading(p, [&](std::istream& is) {
    std::stringstream buf;
    buf << is.rdbuf();
    data = buf.str();
  });
  return data;
}


inline void
write_file(const path& p, const std::string& data)
{
  with_stream_for_writing(p, [&](std::ostream& os) { os << data; });
}

using PathDepthPair = std::pair<path, int>;
using PathDepthPairs = std::vector<PathDepthPair>;

// on windows the user needs special privileges to create symlinks.  In normal development
// mode this privilege is likely not set.  To make sure that unittests don't fail for this
// reason ignore such a privilege condition, but only print it as a warning to stdout.
template <typename F>
void
with_privilege_check(F f)
{
  try {
    f();
  }
  catch (const filesystem_error& fe) {
#if defined(FSPP_IS_WIN)
    if (fe.code().value() == ERROR_PRIVILEGE_NOT_HELD) {
      std::cerr << "TEST disabled: missing privileges" << std::endl;
    }
    else if (fe.code().value() == ERROR_ACCESS_DENIED) {
      std::cerr << "TEST disabled: access denied" << std::endl;
    }
    else {
      throw;
    }
#else
    (void)fe;
    throw;
#endif
  }
}


// in some compilers (gcc4.9 on ubuntu12?) system errors don't seem to be in
// generic_category().  Avoid this detail for the tests here.
inline bool
is_error(std::error_code ec, std::errc errc)
{
  return ec.value() == std::make_error_code(errc).value();
}

}  // namespace tests

}  // namespace filesystem
}  // namespace eyestep


namespace Catch {
template <>
struct StringMaker<std::set<eyestep::filesystem::path>>
{
  static std::string convert(std::set<eyestep::filesystem::path> const& value)
  {
    std::stringstream ss;
    ss << "{";
    bool is_first = true;
    for (const auto& e : value) {
      if (!is_first) {
        ss << ", ";
      }
      else {
        is_first = false;
      }
      ss << "'" << e << "'";
    }
    ss << "}";
    return ss.str();
  }
};


template <>
struct StringMaker<eyestep::filesystem::tests::PathDepthPair>
{
  static std::string convert(eyestep::filesystem::tests::PathDepthPair const& value)
  {
    std::stringstream ss;
    ss << "['" << value.first << "' : " << value.second << "]";
    return ss.str();
  }
};

}  // namespace Catch
