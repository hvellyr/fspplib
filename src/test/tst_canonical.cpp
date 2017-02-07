// Copyright (c) 2016 Gregor Klinke

#include "test_utils.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/platform.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/limits.hpp"
#include "fspp/utils.hpp"

#include <catch/catch.hpp>


namespace eyestep {
namespace filesystem {
namespace tests {

TEST_CASE("canonical", "[operations]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "foo/bar");
    create_directories(root / "moo/var/tmp");
    write_file(root / "moo/var/tmp/file.txt", "hello world");
    write_file(root / "file.txt", "hello world");

    const auto base = root / "foo/bar";

    REQUIRE(equivalent(
      root / "moo/var/tmp/file.txt", canonical(root / "moo/var/tmp/file.txt", base)));
    REQUIRE(equivalent(
      root / "file.txt", canonical(root / "moo/./var/tmp/../../../file.txt", base)));

    auto chdir_guard = make_chdir_scope(root / "foo/bar");

    write_file(u8path("../file.txt"), "hello world");
    REQUIRE(equivalent(root / "foo/file.txt", canonical(u8path("../file.txt"), base)));
  });
}


TEST_CASE("canonical normalizes", "[operations]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "moo/var/tmp");
    const auto base = root / "foo/bar";
    REQUIRE(u8path(".") == canonical(root / "moo/var/tmp/", base).filename());

    REQUIRE(u8path(".") == canonical(root / "moo/var/tmp/.", base).filename());
  });
}


TEST_CASE("canonical with symlinks", "[operations]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      create_directories(root / "foo/bar");

      const auto base = root / "foo/bar";

      write_file(root / "foo/bar/file.txt", "hello world");
      create_directory_symlink(root / "foo/bar", root / "abc");

      REQUIRE(
        equivalent(root / "foo/bar/file.txt", canonical(u8path("abc/file.txt"), root)));
    });
  });
}


TEST_CASE("canonical with symlinks to symlinks", "[operations]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      create_directories(root / "foo/bar");

      write_file(root / "foo/file.txt", "hello world");
      create_directory_symlink(root / "foo/bar", root / "abc1");
      create_directory_symlink(root / "abc1", root / "abc2");
      create_directory_symlink(root / "abc2", root / "abc3");
      REQUIRE(
        equivalent(root / "foo/file.txt", canonical(u8path("abc3/../file.txt"), root)));
    });
  });
}


TEST_CASE("canonical with too many symlinks level", "[operations]")
{
  with_privilege_check([]() {
    const auto max_loop = symlink_loop_maximum();
    if (max_loop <= 33) {
      with_temp_dir([&](const path& root) {
        create_directories(root / "foo/bar");
        write_file(root / "foo/file.txt", "hello world");

        auto last = root / "foo/bar";
        for (auto i = 0u; i < max_loop + 2; ++i) {
          auto next_nm = "abc" + std::to_string(i);
          create_directory_symlink(last, root / next_nm);
          last = root / next_nm;
        }

        std::error_code ec;
        canonical(last / "../file.txt", root, ec);
        REQUIRE(is_error(ec, std::errc::too_many_symbolic_link_levels));
      });
    }
    else {
      std::cerr << "TEST disabled: number of maximum symlink loop is too large to test ("
                << max_loop << ")" << std::endl;
    }
  });
}

}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
