// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file_status.hpp"
#include "fspp/details/platform.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/utils.hpp"

#include "test_utils.hpp"

#include <catch/catch.hpp>

#include <algorithm>
#include <ostream>
#include <random>

#if defined(FSPP_IS_MAC)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


namespace eyestep {
namespace filesystem {
namespace tests {

TEST_CASE("status", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    {
      auto s = status(root / "foo");
      REQUIRE(s.type() == file_type::not_found);
    }

    create_directory(root / "foo");
    permissions(root / "foo", perms::owner_all | perms::group_read | perms::others_read);
    {
      auto s = status(root / "foo");
      REQUIRE(s.type() == file_type::directory);
      REQUIRE(
        (s.permissions() & (perms::owner_all | perms::group_read | perms::others_read))
        != 0);
    }

    touch(root / "bar.txt");
    {
      auto s = status(root / "bar.txt");
      REQUIRE(s.type() == file_type::regular);
      REQUIRE((s.permissions() & perms::owner_all) != 0);
    }
  });
}


TEST_CASE("is_empty", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "abc");
    REQUIRE(is_empty(root / "abc"));

    touch(root / "abc/moo.txt");
    REQUIRE(!is_empty(root / "abc"));

    touch(root / "foo.txt");
    REQUIRE(is_empty(root / "foo.txt"));

    write_file(root / "foo2.txt", "hello world");
    REQUIRE(!is_empty(root / "foo2.txt"));
  });
}


TEST_CASE("last_write_time", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "abc/def/foo");

    with_stream_for_writing(
      root / "abc/def/foo/m1.txt", [](std::ostream& os) { os << "hello world\n"; });
    last_write_time(root / "abc/def/foo/m1.txt", 250);
    REQUIRE(last_write_time(root / "abc/def/foo/m1.txt") == 250);

    with_stream_for_writing(
      root / "abc/def/foo/m2.txt", [](std::ostream& os) { os << "hello world\n"; });

    REQUIRE(last_write_time(root / "abc/def/foo/m2.txt")
            > last_write_time(root / "abc/def/foo/m1.txt"));
  });
}


TEST_CASE("last_write_time in memory vfs", "[operations][emulate-win]")
{
  vfs::with_memory_vfs("//<vfs>", [](vfs::IFilesystem&) {
    auto root = u8path("//<vfs>");
    create_directories(root / "abc/def/foo");

    with_stream_for_writing(
      root / "abc/def/foo/m1.txt", [](std::ostream& os) { os << "hello world\n"; });
    last_write_time(root / "abc/def/foo/m1.txt", 250);
    REQUIRE(last_write_time(root / "abc/def/foo/m1.txt") == 250);

    with_stream_for_writing(
      root / "abc/def/foo/m2.txt", [](std::ostream& os) { os << "hello world\n"; });

    REQUIRE(last_write_time(root / "abc/def/foo/m2.txt")
            > last_write_time(root / "abc/def/foo/m1.txt"));
  });
}


TEST_CASE("symlinks", "[operations][emulate-win]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      create_symlink(root / "usr/eskimo/foo", root / "ixwick");
      REQUIRE(symlink_status(root / "ixwick").type() == file_type::symlink);
      REQUIRE(!exists(root / "ixwick"));
      REQUIRE(read_symlink(root / "ixwick") == root / "/usr/eskimo/foo");
    });
  });
}


TEST_CASE("hardlinks", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });

    REQUIRE(hard_link_count(root / "foo.txt") == 1);

    create_hard_link(root / "foo.txt", root / "bar.txt");
    REQUIRE(hard_link_count(root / "foo.txt") == 2);
    REQUIRE(hard_link_count(root / "bar.txt") == 2);

    remove(root / "foo.txt");
    REQUIRE(hard_link_count(root / "bar.txt") == 1);
  });
}


TEST_CASE("equivalent", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    with_stream_for_writing(
      root / "gaz.txt", [](std::ostream& os) { os << "hello world\n"; });
    create_directory(root / "xar");

    create_hard_link(root / "foo.txt", root / "bar.txt");

    REQUIRE(equivalent(root / "xar", root / "xar"));
    REQUIRE(equivalent(root / "foo.txt", root / "foo.txt"));

    REQUIRE(equivalent(root / "foo.txt", root / "bar.txt"));
    REQUIRE(hard_link_count(root / "gaz.txt") == 1);

    REQUIRE(!equivalent(root / "foo.txt", root / "gaz.txt"));
    REQUIRE(!equivalent(root / "foo.txt", root / "xar"));
  });
}


TEST_CASE("equivalent with symlinks", "[operations][emulate-win]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      with_stream_for_writing(
        root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });

      create_symlink(root / "foo.txt", root / "ixwick");
      REQUIRE(equivalent(root / "foo.txt", root / "ixwick"));
    });
  });
}


TEST_CASE("copy_file", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    permissions(root / "foo.txt", perms::owner_read | perms::owner_write);

    copy_file(root / "foo.txt", root / "bar.txt");

    REQUIRE(exists(root / "bar.txt"));
    REQUIRE(read_file(root / "bar.txt") == "hello world\n");

    REQUIRE(
      (status(root / "bar.txt").permissions() & (perms::owner_read | perms::owner_write))
      != 0);
  });
}


TEST_CASE("copy_file to existing file - none", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    with_stream_for_writing(
      root / "bar.txt", [](std::ostream& os) { os << "annyeong sesang\n"; });

    REQUIRE_THROWS_AS(
      { copy_file(root / "foo.txt", root / "bar.txt"); }, filesystem_error);
  });
}


TEST_CASE("copy_file to existing file - skip_existing", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    with_stream_for_writing(
      root / "bar.txt", [](std::ostream& os) { os << "annyeong sesang\n"; });

    REQUIRE(!copy_file(root / "foo.txt", root / "bar.txt", copy_options::skip_existing));
    REQUIRE(copy_file(root / "foo.txt", root / "gaz.txt", copy_options::skip_existing));

    REQUIRE(exists(root / "bar.txt"));
  });
}


TEST_CASE("copy_file to existing file - overwrite_existing", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    with_stream_for_writing(
      root / "bar.txt", [](std::ostream& os) { os << "annyeong sesang\n"; });

    REQUIRE(
      copy_file(root / "foo.txt", root / "bar.txt", copy_options::overwrite_existing));
    REQUIRE(read_file(root / "bar.txt") == "hello world\n");
  });
}


TEST_CASE("copy_file to existing file - update_existing", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    with_stream_for_writing(
      root / "bar.txt", [](std::ostream& os) { os << "annyeong sesang\n"; });
    with_stream_for_writing(
      root / "moo.txt", [](std::ostream& os) { os << "saluton mondo\n"; });
    last_write_time(root / "foo.txt", 2000);
    last_write_time(root / "bar.txt", 3000);
    last_write_time(root / "moo.txt", 1950);

    REQUIRE(
      !copy_file(root / "foo.txt", root / "bar.txt", copy_options::update_existing));
    REQUIRE(read_file(root / "bar.txt") == "annyeong sesang\n");

    REQUIRE(copy_file(root / "foo.txt", root / "moo.txt", copy_options::update_existing));
    REQUIRE(read_file(root / "moo.txt") == "hello world\n");
  });
}


TEST_CASE("copy_file - fails if target is dir", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    with_stream_for_writing(
      root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
    create_directory(root / "bar.txt");

    REQUIRE_THROWS_AS(
      { copy_file(root / "foo.txt", root / "bar.txt"); }, filesystem_error);

    REQUIRE(is_directory(root / "bar.txt"));
  });
}


TEST_CASE("copy_file - large file", "[operations][emulate-win]")
{
  with_temp_dir([](const path& root) {
    auto data = make_random_string(1 << 16);
    with_stream_for_writing(root / "foo.txt", [&](std::ostream& os) { os << data; });

    copy_file(root / "foo.txt", root / "bar.txt");
    REQUIRE(file_size(root / "bar.txt") == (1 << 16));
    REQUIRE(read_file(root / "bar.txt") == data);
  });
}


TEST_CASE("copy_symlink - to file", "[operations]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      with_stream_for_writing(
        root / "foo.txt", [](std::ostream& os) { os << "hello world\n"; });
      create_directory(root / "abc");
      create_directory(root / "ghi");

      create_symlink("../foo.txt", root / "abc/lnk.txt");
      REQUIRE(read_symlink(root / "abc/lnk.txt") == "../foo.txt");

// it seems that relative symlinks are resolved relative to the current path.  Disable
// that test part for now.
#if !defined(FSPP_IS_WIN)
      REQUIRE(read_file(root / "abc/lnk.txt") == "hello world\n");
#endif

      copy_symlink(root / "abc/lnk.txt", root / "ghi/moo.txt");
      REQUIRE(symlink_status(root / "ghi/moo.txt").type() == file_type::symlink);
      REQUIRE(read_symlink(root / "ghi/moo.txt") == "../foo.txt");

#if !defined(FSPP_IS_WIN)
      REQUIRE(read_file(root / "ghi/moo.txt") == "hello world\n");
#endif
      // this will produce a broken symlink (because the target above is relative)
      copy_symlink(root / "abc/lnk.txt", root / "zap.txt");
#if !defined(FSPP_IS_WIN)
      REQUIRE(!exists(root / "zap.txt"));
#endif
      REQUIRE(read_symlink(root / "zap.txt") == "../foo.txt");

#if !defined(FSPP_IS_WIN)
      REQUIRE_THROWS_AS({ read_file(root / "zap.txt"); }, filesystem_error);
#endif
    });
  });
}


TEST_CASE("copy_symlink - to directory", "[operations]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      create_directory(root / "abc");
      with_stream_for_writing(
        root / "abc/foo.txt", [](std::ostream& os) { os << "hello world\n"; });

      create_symlink(root / "abc", root / "lnk");

      REQUIRE(is_directory(root / "lnk"));
      REQUIRE(is_regular_file(root / "lnk/foo.txt"));

      copy_symlink(root / "lnk", root / "verknuepfung");

      REQUIRE(is_directory(root / "verknuepfung"));
      REQUIRE(is_regular_file(root / "verknuepfung/foo.txt"));
    });
  });
}


TEST_CASE("copy", "[operations][copy][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directory(root / "foo");
    create_directory(root / "bar");

    write_file(root / "foo/aa.txt", "hello world");
    copy(root / "foo/aa.txt", root / "bar/boo.txt");
    REQUIRE(is_regular_file(root / "foo/aa.txt"));
    REQUIRE(is_regular_file(root / "bar/boo.txt"));
    REQUIRE(11 == file_size(root / "bar/boo.txt"));
    REQUIRE(1 == hard_link_count(root / "foo/aa.txt"));
    REQUIRE(1 == hard_link_count(root / "bar/boo.txt"));

    // overwrite the original and make sure that the copy isn't changed
    write_file(root / "foo/aa.txt", "abc");
    REQUIRE(3 == file_size(root / "foo/aa.txt"));
    REQUIRE(11 == file_size(root / "bar/boo.txt"));
  });
}


TEST_CASE("copy - directories", "[operations][copy][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo/bar/gaz");

    write_file(root / "foo/aa.txt", "hello world");
    write_file(root / "foo/bar/b.txt", "annyeong sesang");

    copy(root / "foo", root / "moo");
    REQUIRE(is_directory(root / "foo"));
    REQUIRE(is_directory(root / "moo"));
    REQUIRE(is_regular_file(root / "foo/aa.txt"));
    REQUIRE(file_size(root / "moo/aa.txt") == 11);
    // the default copy options to copy will copy the first
    // level of files only but ignore the folders.
    REQUIRE(!exists(root / "moo/bar"));
    // we really copy and not move
    REQUIRE(!is_empty(root / "foo"));

    // creating a new file in the copied folder shows that the folders are
    // really separate
    write_file(root / "moo/bb.txt", "abc");
    REQUIRE(!is_regular_file(root / "foo/bb.txt"));
    REQUIRE(is_regular_file(root / "moo/bb.txt"));
  });
}


TEST_CASE("copy - directory recursive", "[operations][copy][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo/bar/gaz");

    write_file(root / "foo/aa.txt", "hello world");
    write_file(root / "foo/bar/gaz/nn.txt", "abc");

    copy(root / "foo", root / "moo", copy_options::recursive);
    REQUIRE(!is_empty(root / "foo"));
    REQUIRE(11 == file_size(root / "moo/aa.txt"));
    REQUIRE(3 == file_size(root / "moo/bar/gaz/nn.txt"));
  });
}


TEST_CASE("copy - empty directory recursive", "[operations][copy]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo");

    copy(root / "foo", root / "moo", copy_options::recursive);
    REQUIRE(is_empty(root / "foo"));
    REQUIRE(is_empty(root / "moo"));
  });
}


TEST_CASE("copy - directory recursive overwriting", "[operations][copy]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo/bar/gaz");
    create_directories(root / "moo/bar");

    write_file(root / "foo/aa.txt", "hello world");
    write_file(root / "foo/bar/gaz/nn.txt", "abc");
    write_file(root / "moo/aa.txt", "schnuggel");
    // let there be a unique file in the target folder.
    write_file(root / "moo/oink.txt", "gonzo");

    copy(root / "foo", root / "moo",
         copy_options::recursive | copy_options::overwrite_existing);
    REQUIRE(!is_empty(root / "foo"));
    REQUIRE(11 == file_size(root / "moo/aa.txt"));
    REQUIRE(3 == file_size(root / "moo/bar/gaz/nn.txt"));
    REQUIRE(5 == file_size(root / "moo/oink.txt"));
  });
}


TEST_CASE("copy - directory recursive only dir", "[operations][copy]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo/bar/gaz");

    write_file(root / "foo/aa.txt", "hello world");
    write_file(root / "foo/bar/gaz/nn.txt", "abc");

    copy(root / "foo", root / "moo",
         copy_options::recursive | copy_options::directories_only);
    REQUIRE(!exists(root / "moo/aa.txt"));
    REQUIRE(!exists(root / "moo/bar/gaz/nn.txt"));

    REQUIRE(is_directory(root / "moo/bar/gaz"));
  });
}


TEST_CASE("copy - file to dir", "[operations][copy]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo");
    create_directories(root / "moo");

    write_file(root / "foo/aa.txt", "hello world");
    copy(root / "foo/aa.txt", root / "moo");

    REQUIRE(is_regular_file(root / "moo/aa.txt"));
    REQUIRE(11 == file_size(root / "moo/aa.txt"));
  });
}


TEST_CASE("copy - dir to file", "[operations][copy][emulate-win]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo");
    create_directories(root / "moo");

    write_file(root / "foo/aa.txt", "hello world");

    std::error_code ec;
    copy(root / "moo", root / "foo/aa.txt", ec);

    REQUIRE(is_regular_file(root / "foo/aa.txt"));
    REQUIRE(is_error(ec, std::errc::not_a_directory));
  });
}


TEST_CASE("absolute", "[operations][emulate-win]")
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  auto base = u8path("d:\\Foo\\bar");

  REQUIRE(u8path("c:\\Users") == absolute(u8path("c:\\Users"), base));
  REQUIRE(u8path("c:\\Foo\\bar\\text.txt") == absolute(u8path("c:text.txt"), base));
  REQUIRE(u8path("d:\\var\\tmp\\file.txt")
          == absolute(u8path("\\var\\tmp\\file.txt"), base));
  REQUIRE(u8path("d:\\Foo\\bar\\..\\file.txt") == absolute(u8path("..\\file.txt"), base));

#else
  auto base = u8path("/foo/bar");

  REQUIRE(u8path("/var/tmp/file.txt") == absolute(u8path("/var/tmp/file.txt"), base));
  REQUIRE(u8path("/foo/bar/../file.txt") == absolute(u8path("../file.txt"), base));

#endif
}


TEST_CASE("system_complete", "[operations]")
{
#if defined(FSPP_IS_WIN)
  // It is basically impossible to test system_complete in reliable
  // and safe way.  It use global system-wide state (current path per
  // device), which might be changed by other applications.  The best
  // we can try here is to check that the result is indeed different
  // to the input.
  REQUIRE(u8path("c:\\Users") == system_complete(u8path("c:\\Users")));
  REQUIRE(u8path("c:text.txt") != system_complete(u8path("c:text.txt")));
  REQUIRE(u8path("\\var\\tmp\\file.txt")
          != system_complete(u8path("\\var\\tmp\\file.txt")));
  REQUIRE(u8path("..\\file.txt") != system_complete(u8path("..\\file.txt")));

#else
  REQUIRE(u8path("/var/tmp/file.txt") == system_complete(u8path("/var/tmp/file.txt")));
  REQUIRE(current_path() / "../file.txt" == system_complete(u8path("../file.txt")));

#endif
}

}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
