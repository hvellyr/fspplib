// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file_status.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/utility/scope.hpp"
#include "fspp/utils.hpp"

#include "test_utils.hpp"

#include <catch/catch.hpp>


namespace eyestep {
namespace filesystem {
namespace tests {


TEST_CASE("directory_entry ctor", "[dir-entry]")
{
  auto d = directory_entry(u8path("foo/bar.txt"));
  REQUIRE(d.path() == u8path("foo/bar.txt"));

  d = directory_entry(path());
  REQUIRE(d.path() == path());
}


TEST_CASE("directory_entry assign operators", "[dir-entry]")
{
  auto d = directory_entry(u8path("foo/bar.txt"));
  REQUIRE(d.path() == u8path("foo/bar.txt"));

  d = directory_entry(u8path("moo/vu/dada"));
  REQUIRE(d.path() == u8path("moo/vu/dada"));
}


TEST_CASE("directory_entry assign", "[dir-entry]")
{
  auto d = directory_entry(u8path("foo/bar.txt"));
  d.assign("stuff/gaz.jpg");
  REQUIRE(d.path() == u8path("stuff/gaz.jpg"));
}


TEST_CASE("directory_entry file_size", "[dir-entry]")
{
  with_temp_dir([](const path& root) {
      create_directories(root / "foo");
      write_file(root / "foo/bar.txt", "hello world");
      write_file(root / "foo/gaz.txt", "alas!");

      auto d = directory_entry(root / "foo/bar.txt");
      REQUIRE(11 == d.file_size());
      // the size is explicitely off, to show that the manual set size is used.  This
      // would be of course a programming error.
      d.assign(root / "foo/gaz.txt", 15);
      REQUIRE(15 == d.file_size());
    });
}


TEST_CASE("directory_entry status", "[dir-entry]")
{
  with_temp_dir([](const path& root) {
      create_directories(root / "foo");
      write_file(root / "foo/bar.txt", "hello world");

      auto d = directory_entry(root / "foo/bar.txt");
      REQUIRE(file_type::regular == d.status().type());
    });
}


TEST_CASE("directory_entry comparision operators", "[dir-entry]")
{
  auto d1 = directory_entry(u8path("foo/bar.txt"));
  auto d2 = directory_entry(u8path("gaz/bar.txt"));

  REQUIRE(d1 < d2);
  REQUIRE(d1 <= d2);
  REQUIRE(d2 > d1);
  REQUIRE(d2 >= d1);

  REQUIRE(d1 == d1);
  REQUIRE(d1 >= d1);
  REQUIRE(d1 <= d1);
  REQUIRE(d1 != d2);
}

}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
