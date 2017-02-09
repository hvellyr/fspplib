// Copyright (c) 2016 Gregor Klinke

#include "fspp/filesystem.hpp"

#include <catch/catch.hpp>

#include <iostream>


namespace eyestep {
namespace tests {

namespace fs = eyestep::filesystem;

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
#define WSEP L"\\"
#define SEP "\\"
#define WSEPC L'\\'
#define SEPC '\\'
#else
#define WSEP L"/"
#define SEP "/"
#define WSEPC L'/'
#define SEPC '/'
#endif


TEST_CASE("ctors", "[path][emulate-win]")
{
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc" SEP "foo").string());
#if defined(FSPP_SUPPORT_WSTRING_API)
  REQUIRE(std::string("abc" SEP "foo") == fs::path(L"abc" WSEP L"foo").string());
#endif
  REQUIRE(std::string() == fs::path().string());
}


TEST_CASE("iterator ctors", "[path][emulate-win]")
{
  using std::begin;
  using std::end;

  auto v = std::vector<char>{'a', 'b', 'c', SEPC, 'f', 'o', 'o'};
  REQUIRE(std::string("abc" SEP "foo") == fs::path(begin(v), end(v)).string());

#if defined(FSPP_SUPPORT_WSTRING_API)
  auto wv = std::vector<wchar_t>{L'a', L'b', L'c', WSEPC, L'f', L'o', L'o'};
  REQUIRE(std::string("abc" SEP "foo") == fs::path(begin(wv), end(wv)).string());
#endif
}


TEST_CASE("string()", "[path][emulate-win]")
{
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc" SEP "foo").string());
#if defined(FSPP_SUPPORT_WSTRING_API)
  REQUIRE(std::string("abc" SEP "foo") == fs::path(L"abc" WSEP L"foo").string());
#endif
}


TEST_CASE("native()", "[path][emulate-win]")
{
#if defined(FSPP_IS_WIN)
  REQUIRE(std::wstring(L"abc\\foo") == fs::path("abc\\foo").native());
  REQUIRE(std::wstring(L"abc\\foo") == fs::path(L"abc\\foo").native());
#else
  REQUIRE(std::string("abc/foo") == fs::path("abc/foo").native());
#if defined(FSPP_SUPPORT_WSTRING_API)
  REQUIRE(std::string("abc/foo") == fs::path(L"abc/foo").native());
#endif
#endif
}


TEST_CASE("u8string()", "[path][emulate-win]")
{
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc" SEP "foo").u8string());
#if defined(FSPP_SUPPORT_WSTRING_API)
  REQUIRE(std::string("abc" SEP "foo") == fs::path(L"abc" WSEP L"foo").u8string());
#endif
}


// TODO: u8string with unicode characters


TEST_CASE("string cast operator", "[path][emulate-win]")
{
#if defined(FSPP_IS_WIN)
  std::wstring s = fs::path(L"abc\\foo");
  REQUIRE(std::wstring(L"abc\\foo") == s);
#else
  std::string s = fs::path("abc/foo");
  REQUIRE(std::string("abc/foo") == s);
#endif
}


TEST_CASE("generic_string", "[path][emulate-win]")
{
  REQUIRE(std::string("abc/foo") == fs::path("abc/foo").generic_string());
  REQUIRE(std::string("abc/foo") == fs::path("abc\\foo").generic_string());
}


TEST_CASE("append", "[path][emulate-win]")
{
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc").append("foo").string());
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc" SEP).append("foo").string());
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc").append(SEP "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == fs::path("").append("abc" SEP "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == fs::path("abc" SEP "foo").append("").string());

  REQUIRE(std::string("abc" SEP "foo")
          == fs::path("abc").append(fs::path("foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == fs::path("abc" SEP).append(fs::path("foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == fs::path("abc").append(fs::path(SEP "foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == fs::path("").append(fs::path("abc" SEP "foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == fs::path("abc" SEP "foo").append(fs::path("")).string());

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  REQUIRE(std::string("d:foo") == fs::path("d:").append("foo").string());
  REQUIRE(std::string("d:\\foo") == fs::path("d:\\").append("foo").string());
  REQUIRE(std::string("d:abc\\foo") == fs::path("d:abc").append("foo").string());

  REQUIRE(std::string("\\\\abc\\foo") == fs::path("\\\\abc").append("foo").string());
#else
  REQUIRE(std::string("/abc/foo") == fs::path("/").append("abc").append("foo").string());
#endif
}


TEST_CASE("operator/", "[path][emulate-win]")
{
  REQUIRE(std::string("abc" SEP "foo") == (fs::path("abc") / "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == (fs::path("abc" SEP) / "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == (fs::path("abc") / SEP "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == (fs::path("") / "abc" SEP "foo").string());
  REQUIRE(std::string("abc" SEP "foo") == (fs::path("abc" SEP "foo") / "").string());

  REQUIRE(std::string("abc" SEP "foo") == (fs::path("abc") / fs::path("foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == (fs::path("abc" SEP) / fs::path("foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == (fs::path("abc") / fs::path(SEP "foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == (fs::path("") / fs::path("abc" SEP "foo")).string());
  REQUIRE(std::string("abc" SEP "foo")
          == (fs::path("abc" SEP "foo") / fs::path("")).string());

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  REQUIRE(std::string("d:foo") == (fs::path("d:") / "foo").string());
  REQUIRE(std::string("d:\\foo") == (fs::path("d:\\") / "foo").string());
  REQUIRE(std::string("d:abc\\foo") == (fs::path("d:abc") / "foo").string());

  REQUIRE(std::string("\\\\abc\\foo") == (fs::path("\\\\abc") / "foo").string());
#else
  REQUIRE(std::string("/abc/foo") == (fs::path("/") / "abc" / "foo").string());
#endif
}

// TODO: operator/=


TEST_CASE("append with iterators", "[path][emulate-win]")
{
  auto v = std::vector<char>{'a', 'b', 'c', SEPC, 'f', 'o', 'o'};
  REQUIRE(std::string("dix" SEP "abc" SEP "foo")
          == fs::path("dix").append(begin(v), end(v)).string());

#if defined(FSPP_SUPPORT_WSTRING_API)
  auto wv = std::vector<wchar_t>{L'a', L'b', L'c', WSEPC, L'f', L'o', L'o'};
  REQUIRE(std::string("dix" SEP "abc" SEP "foo")
          == fs::path("dix").append(begin(wv), end(wv)).string());
#endif
}


TEST_CASE("operator+=", "[path][emulate-win]")
{
  REQUIRE(std::string("abcfoo" SEP "dix")
          == (fs::path("abc") += fs::path("foo" SEP "dix")).string());
  REQUIRE(std::string("abcfoo/dix") == (fs::path("abc") += "foo/dix").string());
  REQUIRE(std::string("abcfoo/dix")
          == (fs::path("abc") += std::string("foo/dix")).string());
#if defined(FSPP_SUPPORT_WSTRING_API)
  REQUIRE(std::string("abcfoo") == (fs::path("abc") += L"foo").string());
  REQUIRE(std::string("abcf") == (fs::path("abc") += L'f').string());
  REQUIRE(std::string("abc" SEP) == (fs::path("abc") += WSEPC).string());
#endif
}


TEST_CASE("make_preferred", "[path][emulate-win]")
{
  REQUIRE(std::string("c:" SEP "abc" SEP "foo" SEP)
          == fs::path("c:/abc/foo/").make_preferred().string());
  REQUIRE(std::string() == fs::path().make_preferred().string());
}


TEST_CASE("iterator", "[path][iterator][emulate-win]")
{
  using namespace std;
  using std::begin;
  using std::end;

  auto check_it = [](const fs::path& p, const vector<string>& exp, const char* desc) {
    SECTION(desc)
    {
      auto v = vector<string>{};
      for (auto x : p) {
        v.emplace_back(x.string());
      }
      REQUIRE(exp == v);
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:", {"c:"}, "c:");
  check_it("c:/", {"c:", "/"}, "c:/");
  check_it("c:foo", {"c:", "foo"}, "c:foo");
  check_it("c:/foo", {"c:", "/", "foo"}, "c:/foo");
  check_it("c:/foo/bar", {"c:", "/", "foo", "bar"}, "c:/foo/bar");
  check_it("prn:", {"prn:"}, "prn:");
  check_it("c:\\", {"c:", "\\"}, "c:\\");
  check_it("c:\\foo", {"c:", "\\", "foo"}, "c:\\foo");
  check_it("c:\\foo\\bar", {"c:", "\\", "foo", "bar"}, "c:\\foo\\bar");
  check_it("c:/foo\\bar", {"c:", "\\", "foo", "bar"}, "c:\\foo\\bar");

  check_it("", {}, "empty path");
  check_it(".", {"."}, ".");
  check_it("..", {".."}, "..");
  check_it("foo", {"foo"}, "foo");
  check_it("/", {"/"}, "/");
  check_it("/foo", {"/", "foo"}, "/foo");
  check_it("foo/", {"foo", "."}, "foo/");
  check_it("/foo/", {"/", "foo", "."}, "/foo/");
  check_it("foo/bar", {"foo", "bar"}, "foo/bar");
  check_it("/foo/bar", {"/", "foo", "bar"}, "/foo/bar");
  check_it("//net", {"//net"}, "//net");
  check_it("//net/foo", {"//net", "/", "foo"}, "//net/foo");
  check_it("///foo///", {"/", "foo", "."}, "///foo///");
  check_it("///foo///bar", {"/", "foo", "bar"}, "///foo///bar");
  check_it("/.", {"/", "."}, "/.");
  check_it("./", {".", "."}, "./");
  check_it("/..", {"/", ".."}, "/..");
  check_it("../", {"..", "."}, "../");
  check_it("foo/.", {"foo", "."}, "foo/.");
  check_it("foo/..", {"foo", ".."}, "foo/..");
  check_it("foo/./", {"foo", ".", "."}, "foo/./");
  check_it("foo/./bar", {"foo", ".", "bar"}, "foo/./bar");
  check_it("foo/../bar", {"foo", "..", "bar"}, "foo/../bar");

#else

  check_it("c:", {"c:"}, "c:");
  check_it("c:/", {"c:", "."}, "c:/");
  check_it("c:foo", {"c:foo"}, "c:foo");
  check_it("c:/foo", {"c:", "foo"}, "c:/foo");
  check_it("c:/foo/bar", {"c:", "foo", "bar"}, "c:/foo/bar");
  check_it("prn:", {"prn:"}, "prn:");
  check_it("c:\\", {"c:\\"}, "c:\\");
  check_it("c:\\foo", {"c:\\foo"}, "c:\\foo");
  check_it("c:\\foo\\bar", {"c:\\foo\\bar"}, "c:\\foo\\bar");
  check_it("c:/foo\\bar", {"c:", "foo\\bar"}, "c:/foo\\bar");

  check_it("", {}, "empty path");
  check_it(".", {"."}, ".");
  check_it("..", {".."}, "..");
  check_it("foo", {"foo"}, "foo");
  check_it("/", {"/"}, "/");
  check_it("/foo", {"/", "foo"}, "/foo");
  check_it("foo/", {"foo", "."}, "foo/");
  check_it("/foo/", {"/", "foo", "."}, "/foo/");
  check_it("foo/bar", {"foo", "bar"}, "foo/bar");
  check_it("/foo/bar", {"/", "foo", "bar"}, "/foo/bar");
  check_it("//net", {"//net"}, "//net");
  check_it("//net/foo", {"//net", "/", "foo"}, "//net/foo");
  check_it("///foo///", {"/", "foo", "."}, "///foo///");
  check_it("///foo///bar", {"/", "foo", "bar"}, "///foo///bar");
  check_it("/.", {"/", "."}, "/.");
  check_it("./", {".", "."}, "./");
  check_it("/..", {"/", ".."}, "/..");
  check_it("../", {"..", "."}, "../");
  check_it("foo/.", {"foo", "."}, "foo/.");
  check_it("foo/..", {"foo", ".."}, "foo/..");
  check_it("foo/./", {"foo", ".", "."}, "foo/./");
  check_it("foo/./bar", {"foo", ".", "bar"}, "foo/./bar");
  check_it("foo/../bar", {"foo", "..", "bar"}, "foo/../bar");

#endif
}


TEST_CASE("reverse_iterator", "[path][iterator][emulate-win]")
{
  using namespace std;
  using std::begin;
  using std::end;

  auto check_it = [](const fs::path& p, const vector<string>& exp, const char* desc) {
    SECTION(std::string(desc) + " reverse")
    {
      auto v2 = vector<string>{};
      for (auto it = end(p), i_first = begin(p); distance(i_first, it) > 0; --it) {
        v2.emplace_back(prev(it)->string());
      }
      std::reverse(begin(v2), end(v2));
      REQUIRE(exp == v2);
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:", {"c:"}, "c:");
  check_it("c:/", {"c:", "/"}, "c:/");
  check_it("c:foo", {"c:", "foo"}, "c:foo");
  check_it("c:/foo", {"c:", "/", "foo"}, "c:/foo");
  check_it("c:/foo/bar", {"c:", "/", "foo", "bar"}, "c:/foo/bar");
  check_it("prn:", {"prn:"}, "prn:");
  check_it("c:\\", {"c:", "\\"}, "c:\\");
  check_it("c:\\foo", {"c:", "\\", "foo"}, "c:\\foo");
  check_it("c:\\foo\\bar", {"c:", "\\", "foo", "bar"}, "c:\\foo\\bar");
  check_it("c:/foo\\bar", {"c:", "\\", "foo", "bar"}, "c:\\foo\\bar");

  check_it("", {}, "empty path");
  check_it(".", {"."}, ".");
  check_it("..", {".."}, "..");
  check_it("foo", {"foo"}, "foo");
  check_it("/", {"/"}, "/");
  check_it("/foo", {"/", "foo"}, "/foo");
  check_it("foo/", {"foo", "."}, "foo/");
  check_it("/foo/", {"/", "foo", "."}, "/foo/");
  check_it("foo/bar", {"foo", "bar"}, "foo/bar");
  check_it("/foo/bar", {"/", "foo", "bar"}, "/foo/bar");
  check_it("//net", {"//net"}, "//net");
  check_it("//net/foo", {"//net", "/", "foo"}, "//net/foo");
  check_it("///foo///", {"/", "foo", "."}, "///foo///");
  check_it("///foo///bar", {"/", "foo", "bar"}, "///foo///bar");
  check_it("/.", {"/", "."}, "/.");
  check_it("./", {".", "."}, "./");
  check_it("/..", {"/", ".."}, "/..");
  check_it("../", {"..", "."}, "../");
  check_it("foo/.", {"foo", "."}, "foo/.");
  check_it("foo/..", {"foo", ".."}, "foo/..");
  check_it("foo/./", {"foo", ".", "."}, "foo/./");
  check_it("foo/./bar", {"foo", ".", "bar"}, "foo/./bar");
  check_it("foo/../bar", {"foo", "..", "bar"}, "foo/../bar");

#else

  check_it("c:", {"c:"}, "c:");
  check_it("c:/", {"c:", "."}, "c:/");
  check_it("c:foo", {"c:foo"}, "c:foo");
  check_it("c:/foo", {"c:", "foo"}, "c:/foo");
  check_it("c:/foo/bar", {"c:", "foo", "bar"}, "c:/foo/bar");
  check_it("prn:", {"prn:"}, "prn:");
  check_it("c:\\", {"c:\\"}, "c:\\");
  check_it("c:\\foo", {"c:\\foo"}, "c:\\foo");
  check_it("c:\\foo\\bar", {"c:\\foo\\bar"}, "c:\\foo\\bar");
  check_it("c:/foo\\bar", {"c:", "foo\\bar"}, "c:/foo\\bar");

  check_it("", {}, "empty path");
  check_it(".", {"."}, ".");
  check_it("..", {".."}, "..");
  check_it("foo", {"foo"}, "foo");
  check_it("/", {"/"}, "/");
  check_it("/foo", {"/", "foo"}, "/foo");
  check_it("foo/", {"foo", "."}, "foo/");
  check_it("/foo/", {"/", "foo", "."}, "/foo/");
  check_it("foo/bar", {"foo", "bar"}, "foo/bar");
  check_it("/foo/bar", {"/", "foo", "bar"}, "/foo/bar");
  check_it("//net", {"//net"}, "//net");
  check_it("//net/foo", {"//net", "/", "foo"}, "//net/foo");
  check_it("///foo///", {"/", "foo", "."}, "///foo///");
  check_it("///foo///bar", {"/", "foo", "bar"}, "///foo///bar");
  check_it("/.", {"/", "."}, "/.");
  check_it("./", {".", "."}, "./");
  check_it("/..", {"/", ".."}, "/..");
  check_it("../", {"..", "."}, "../");
  check_it("foo/.", {"foo", "."}, "foo/.");
  check_it("foo/..", {"foo", ".."}, "foo/..");
  check_it("foo/./", {"foo", ".", "."}, "foo/./");
  check_it("foo/./bar", {"foo", ".", "bar"}, "foo/./bar");
  check_it("foo/../bar", {"foo", "..", "bar"}, "foo/../bar");

#endif
}


TEST_CASE("compare ==", "[path][emulate-win]")
{
  REQUIRE(fs::path("/foo/bar") == fs::path("/foo/bar"));
  REQUIRE(fs::path() == fs::path());
  REQUIRE(fs::path("") == fs::path());
  REQUIRE(fs::path("/") == fs::path("/"));
  REQUIRE(fs::path("/foo/") == fs::path("/foo/."));
  REQUIRE(fs::path("///foo//bar") == fs::path("/foo/bar"));
}


TEST_CASE("compare !=", "[path][emulate-win]")
{
  REQUIRE(fs::path("foo/bar") != fs::path("/foo/bar"));
  REQUIRE(fs::path("a") != fs::path("b"));
  REQUIRE(fs::path("a") != fs::path());
}


TEST_CASE("compare <", "[path][emulate-win]")
{
  REQUIRE(fs::path("/foo/bar") < fs::path("/foo/barx"));
  REQUIRE(fs::path("/foo/bar") < fs::path("/foox/bar"));
  REQUIRE(fs::path("/foo/bar") < fs::path("a"));
}


TEST_CASE("compare <=", "[path][emulate-win]")
{
  REQUIRE(fs::path("/foo/bar") <= fs::path("/foo/barx"));
  REQUIRE(fs::path("/foo/bar") <= fs::path("/foox/bar"));
  REQUIRE(fs::path("/foo/bar") <= fs::path("/foo/bar"));
}


TEST_CASE("compare >", "[path][emulate-win]")
{
  REQUIRE(fs::path("/foo/bar") > fs::path());
  REQUIRE(fs::path("/foo/bar") > fs::path("/a"));
  REQUIRE(fs::path("/foox/bar") > fs::path("/foo/bar/x"));
}


TEST_CASE("compare >=", "[path][emulate-win]")
{
  REQUIRE(fs::path("/foo/bar") >= fs::path());
  REQUIRE(fs::path("/foo/bar") >= fs::path("/a"));
  REQUIRE(fs::path("/foox/bar") >= fs::path("/foox/bar"));
}


//----------------------------------------------------------------------------------------

TEST_CASE("root_name", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " root name")
    {
      REQUIRE(p.has_root_name() == expq);
      REQUIRE(exp == p.root_name());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it(L"C:/foo/bar", true, L"C:");
  check_it(L"Z:\\foo\\bar", true, L"Z:");
  check_it(L"\\\\net\\foo\\bar", true, L"\\\\net");
#endif

  check_it("/foo/bar", false, {});
  check_it("a/foo/bar", false, {});
  check_it("//net/foo/bar", true, "//net");
}


TEST_CASE("root_directory", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " root directory")
    {
      REQUIRE(p.has_root_directory() == expq);
      REQUIRE(exp == p.root_directory());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it(L"C:/foo/bar", true, "/");
  check_it(L"Z:", false, {});
  check_it(L"\\\\net\\foo\\bar", true, "\\");
  check_it(L"//net/foo/bar", true, "/");
  check_it(L"\\\\net", false, {});
  check_it(L"/.", true, "/");
  check_it(L"/foo", true, "/");
  check_it(L"\\foo", true, "\\");
  check_it(L".", false, {});
  check_it(L"foo", false, {});
#else
  check_it("C:/foo/bar", false, {});
  check_it("Z:", false, {});
  check_it("\\net\foo\bar", false, {});
  check_it("//net/foo/bar", true, "/");
  check_it("//net", false, {});
  check_it("/.", true, "/");
  check_it("/foo", true, "/");
  check_it("\\foo", false, {});
  check_it(".", false, {});
  check_it("foo", false, {});
#endif
}


TEST_CASE("root_path", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " root_path")
    {
      REQUIRE(p.has_root_path() == expq);
      REQUIRE(exp == p.root_path());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:/foo", true, "c:/");
  check_it("c:\\foo", true, "c:\\");
#else
  check_it("c:/foo", false, {});
#endif

  check_it({}, false, {});
  check_it("foo", false, {});
  check_it("/", true, "/");
  check_it("/foo", true, "/");
  check_it("//net", true, "//net");
  check_it("//net/", true, "//net/");
  check_it("//net/bar", true, "//net/");
}


TEST_CASE("relative_path", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " relative path")
    {
      REQUIRE(p.has_relative_path() == expq);
      REQUIRE(exp == p.relative_path());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:", false, {});
  check_it("c:/", false, {});
  check_it("c:foo", true, "foo");
  check_it("c:/foo", true, "foo");
  check_it("c:/foo/bar", true, "foo/bar");
  check_it("c:\\", false, {});
  check_it("c:\\foo", true, "foo");
  check_it("c:\\foo\\bar", true, "foo\\bar");
  check_it("c:/foo\\bar", true, "foo\\bar");

  // Boost filesystem treats "prn:" as a root, it's unclear though whether
  // this is (1) useful or (2) feasible.  There's no relative path or anything
  // else valid after a "prn:" (or "lpt2:" or any of the other reserved device
  // names).
  // check_it("prn:", false, {});
  check_it("prn:", true, "prn:");

#else
  check_it("c:", true, "c:");
  check_it("c:/", true, "c:/");
  check_it("c:foo", true, "c:foo");
  check_it("c:/foo", true, "c:/foo");
  check_it("c:/foo/bar", true, "c:/foo/bar");
  check_it("prn:", true, "prn:");
  check_it("c:\\", true, "c:\\");
  check_it("c:\\foo", true, "c:\\foo");
  check_it("c:\\foo\\bar", true, "c:\\foo\\bar");
  check_it("c:/foo\\bar", true, "c:/foo\\bar");

#endif

  check_it({}, false, {});
  check_it(".", true, ".");
  check_it("..", true, "..");
  check_it("foo", true, "foo");
  check_it("/", false, {});
  check_it("/foo", true, "foo");
  check_it("foo/", true, "foo/");
  check_it("/foo/", true, "foo/");
  check_it("foo/bar", true, "foo/bar");
  check_it("/foo/bar", true, "foo/bar");
  check_it("//net", false, {});
  check_it("//net/foo", true, "foo");
  check_it("///foo///", true, "foo///");
  check_it("///foo///bar", true, "foo///bar");
  check_it("/.", true, ".");
  check_it("./", true, "./");
  check_it("/..", true, "..");
  check_it("../", true, "../");
  check_it("foo/.", true, "foo/.");
  check_it("foo/..", true, "foo/..");
  check_it("foo/./", true, "foo/./");
  check_it("foo/./bar", true, "foo/./bar");
  check_it("foo/../bar", true, "foo/../bar");
}


TEST_CASE("filename", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " filename")
    {
      REQUIRE(p.has_filename() == expq);
      REQUIRE(exp == p.filename());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:/", true, "/");
  check_it("c:foo", true, "foo");
  check_it("c:/foo", true, "foo");
  check_it("c:/foo/bar", true, "bar");
  check_it("prn:", true, "prn:");
  check_it("c:\\", true, "\\");
  check_it("c:\\foo", true, "foo");
  check_it("c:\\foo\\bar", true, "bar");
  check_it("c:/foo\\bar", true, "bar");

#else
  check_it("c:/", true, ".");
  check_it("c:foo", true, "c:foo");
  check_it("c:/foo", true, "foo");
  check_it("c:/foo/bar", true, "bar");
  check_it("prn:", true, "prn:");
  check_it("c:\\", true, "c:\\");
  check_it("c:\\foo", true, "c:\\foo");
  check_it("c:\\foo\\bar", true, "c:\\foo\\bar");
  check_it("c:/foo\\bar", true, "foo\\bar");

#endif

  check_it("c:", true, "c:");
  check_it({}, false, {});
  check_it(".", true, ".");
  check_it("..", true, "..");
  check_it("foo", true, "foo");
  check_it("/", true, "/");
  check_it("/foo", true, "foo");
  check_it("foo/", true, ".");
  check_it("/foo/", true, ".");
  check_it("foo/bar", true, "bar");
  check_it("/foo/bar", true, "bar");
  check_it("//net", true, "//net");
  check_it("//net/foo", true, "foo");
  check_it("///foo///", true, ".");
  check_it("///foo///bar", true, "bar");
  check_it("/.", true, ".");
  check_it("./", true, ".");
  check_it("/..", true, "..");
  check_it("../", true, ".");
  check_it("foo/.", true, ".");
  check_it("foo/..", true, "..");
  check_it("foo/./", true, ".");
  check_it("foo/./bar", true, "bar");
  check_it("foo/../bar", true, "bar");
}


TEST_CASE("parent_path", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " parent_path")
    {
      REQUIRE(p.has_parent_path() == expq);
      REQUIRE(exp == p.parent_path());
    }
  };

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  check_it("c:/", true, "c:");
  check_it("c:foo", true, "c:");
  check_it("c:/foo", true, "c:/");
  check_it("c:/foo/bar", true, "c:/foo");
  check_it("c:\\", true, "c:");
  check_it("c:\\foo", true, "c:\\");
  check_it("c:\\foo\\bar", true, "c:\\foo");
  check_it("c:/foo\\bar", true, "c:/foo");

#else
  check_it("c:foo", false, {});
  check_it("c:/foo", true, "c:");
  check_it("c:\\", false, {});
  check_it("c:\\foo", false, {});
  check_it("c:\\foo\\bar", false, {});
  check_it("c:/foo\\bar", true, "c:");

#endif

  check_it("c:", false, {});
  check_it("c:/", true, "c:");
  check_it("c:/foo/bar", true, "c:/foo");
  check_it("prn:", false, {});
  check_it({}, false, {});
  check_it(".", false, {});
  check_it("..", false, {});
  check_it("foo", false, {});
  check_it("/", false, {});
  check_it("/foo", true, "/");
  check_it("foo/", true, "foo");
  check_it("/foo/", true, "/foo");
  check_it("foo/bar", true, "foo");
  check_it("/foo/bar", true, "/foo");
  check_it("//net", false, {});
  check_it("//net/foo", true, "//net/");
  check_it("///foo///", true, "///foo");
  check_it("///foo///bar", true, "///foo");
  check_it("/.", true, "/");
  check_it("./", true, ".");
  check_it("/..", true, "/");
  check_it("../", true, "..");
  check_it("foo/.", true, "foo");
  check_it("foo/..", true, "foo");
  check_it("foo/./", true, "foo/.");
  check_it("foo/./bar", true, "foo/.");
  check_it("foo/../bar", true, "foo/..");
}


TEST_CASE("stem", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " stem")
    {
      REQUIRE(p.has_stem() == expq);
      REQUIRE(exp == p.stem());
    }
  };

  check_it("/foo/bar", true, "bar");
  check_it("/foo/bar.txt", true, "bar");
  check_it("/foo/bar.1999.txt", true, "bar.1999");
  check_it("/foo/bar..txt", true, "bar.");
  check_it("/foo/bar.txt/gaz", true, "gaz");
  check_it("/foo/.hidden", false, {});
  check_it("/foo/bar.", true, "bar");
  check_it("/foo/.", true, ".");
  check_it("/foo/..", true, "..");
  check_it("foo/", true, ".");
}


TEST_CASE("extension", "[path][emulate-win]")
{
  const auto check_it = [](const fs::path& p, bool expq, const fs::path& exp) {
    SECTION(p.string() + " extension")
    {
      REQUIRE(p.has_extension() == expq);
      REQUIRE(exp == p.extension());
    }
  };

  check_it("/foo/bar", false, {});
  check_it("/foo/bar.txt", true, ".txt");
  check_it("/foo/bar.1999.txt", true, ".txt");
  check_it("/foo/bar..txt", true, ".txt");
  check_it("/foo/bar.txt/gaz", false, {});
  check_it("/foo/.hidden", true, ".hidden");
  check_it("/foo/bar.", true, ".");
  check_it("/foo/.", false, {});
  check_it("/foo/..", false, {});
  check_it("foo/", false, {});
}


TEST_CASE("is_absolute", "[path][emulate-win]")
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  REQUIRE(!fs::path("/foo").is_absolute());
  REQUIRE(fs::path("c:/foo").is_absolute());
#else
  REQUIRE(fs::path("/foo").is_absolute());
  REQUIRE(!fs::path("c:/foo").is_absolute());
#endif
  REQUIRE(fs::path("//net/foo").is_absolute());

  REQUIRE(!fs::path("foo/bar").is_absolute());
}


TEST_CASE("remove_filename", "[path][emulate-win]")
{
  SECTION("returns the changed value")
  {
    REQUIRE(fs::path("/") == fs::path("/foo").remove_filename());
    REQUIRE(fs::path() == fs::path("/").remove_filename());
  }

  SECTION("is modifying the value")
  {
    auto t = fs::path("/foo");
    t.remove_filename();
    REQUIRE(fs::path("/") == t);
  }
}


TEST_CASE("replace_filename", "[path][emulate-win]")
{
  SECTION("returns the changed value")
  {
    REQUIRE(fs::path(SEP "bar") == fs::path(SEP "foo").replace_filename("bar"));
    REQUIRE(fs::path("bar") == fs::path("/").replace_filename("bar"));
    REQUIRE(fs::path("//net" SEP "bar") == fs::path("//net/").replace_filename("bar"));
  }

  SECTION("is modifying the value")
  {
    auto t = fs::path(SEP "foo");
    t.replace_filename("bar");
    REQUIRE(fs::path(SEP "bar") == t);
  }
}


TEST_CASE("replace_extension", "[path][emulate-win]")
{
  SECTION("replacement without leading dot")
  {
    REQUIRE(fs::path(SEP "bar.jpeg")
            == fs::path(SEP "bar.jpg").replace_extension("jpeg"));
    REQUIRE(fs::path(SEP "bar.jpeg") == fs::path(SEP "bar").replace_extension("jpeg"));
    REQUIRE(fs::path(SEP "bar.jpeg") == fs::path(SEP "bar.").replace_extension("jpeg"));
    REQUIRE(fs::path(SEP "bar..jpeg")
            == fs::path(SEP "bar..txt").replace_extension("jpeg"));

    auto t = fs::path(SEP "bar.jpg");
    t.replace_extension("jpeg");
    REQUIRE(fs::path(SEP "bar.jpeg") == t);
  }

  SECTION("replacement with leading dot")
  {
    REQUIRE(fs::path(SEP "bar.jpeg")
            == fs::path(SEP "bar.jpg").replace_extension(".jpeg"));
    REQUIRE(fs::path(SEP "bar.jpeg") == fs::path(SEP "bar").replace_extension(".jpeg"));
    REQUIRE(fs::path(SEP "bar.jpeg") == fs::path(SEP "bar.").replace_extension(".jpeg"));
    REQUIRE(fs::path(SEP "bar..jpeg")
            == fs::path(SEP "bar..txt").replace_extension(".jpeg"));

    auto t = fs::path(SEP "bar.jpg");
    t.replace_extension(".jpeg");
    REQUIRE(fs::path(SEP "bar.jpeg") == t);
  }

  SECTION("empty replacement")
  {
    REQUIRE(fs::path(SEP "bar") == fs::path(SEP "bar.jpg").replace_extension());
    REQUIRE(fs::path(SEP "bar") == fs::path(SEP "bar").replace_extension());
    REQUIRE(fs::path(SEP "bar") == fs::path(SEP "bar.").replace_extension({}));
    REQUIRE(fs::path(SEP "bar.") == fs::path(SEP "bar..txt").replace_extension());

    auto t = fs::path(SEP "bar.jpg");
    t.replace_extension();
    REQUIRE(fs::path(SEP "bar") == t);
  }
}


TEST_CASE("lexical_normal", "[path][emulate-win]")
{
  REQUIRE(fs::path("foo/./bar/..").lexically_normal() == "foo");
  REQUIRE(fs::path("foo/.///bar/../").lexically_normal() == "foo/.");
  REQUIRE(fs::path("foo/../..//bar/../").lexically_normal() == "../.");
  REQUIRE(fs::path("foo/./bar/.").lexically_normal() == "foo/bar/.");
}


TEST_CASE("lexical_relative", "[path][emulate-win]")
{
  REQUIRE(fs::path("/a/d").lexically_relative("/a/b/c") == "../../d");
  REQUIRE(fs::path("/a/b/c").lexically_relative("/a/d") == "../b/c");
  REQUIRE(fs::path("a/b/c").lexically_relative("a") == "b/c");
  REQUIRE(fs::path("a/b/c").lexically_relative("a/b/c/x/y") == "../..");
  REQUIRE(fs::path("a/b/c").lexically_relative("a/b/c") == ".");
  REQUIRE(fs::path("a/b").lexically_relative("c/d") == "");
  REQUIRE(fs::path("/a/b").lexically_relative("c/d") == "");
  REQUIRE(fs::path("/a/b").lexically_relative("/a/b") == ".");
}


TEST_CASE("lexical_proximate", "[path][emulate-win]")
{
  REQUIRE(fs::path("/a/d").lexically_proximate("/a/b/c") == "../../d");
  REQUIRE(fs::path("a/b").lexically_proximate("c/d") == "a/b");
}

}  // namespace tests
}  // namespace eyestep
