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


TEST_CASE("weakly_canonical", "[operations]")
{
  with_temp_dir([&](const path& root) {
    const auto canonical_root = canonical(root);
    create_directories(root / "foo");

    REQUIRE(u8path("foo/abc/file.txt")
            == weakly_canonical(root / "foo/abc/bar/../file.txt")
                 .lexically_relative(canonical_root));

    // the return value is a path in normal form
    REQUIRE(u8path("foo/abc/gaz/.")
            == weakly_canonical(root / "foo/abc/bar/./moo//../../gaz/.")
                 .lexically_relative(canonical_root));
  });
}


TEST_CASE("weakly_canonical with all elements existing", "[operations]")
{
  with_temp_dir([&](const path& root) {
    const auto canonical_root = canonical(root);
    create_directories(root / "foo/bar");
    write_file(root / "foo/file.txt", "hello world");

    REQUIRE(u8path("foo/file.txt")
            == weakly_canonical(root / "foo/bar/../file.txt")
                 .lexically_relative(canonical_root));
  });
}


TEST_CASE("weakly_canonical with symlinks", "[operations][gck]")
{
#if defined(FSPP_IS_WIN)
  with_privilege_check([]() {
    with_temp_dir([&](const path& root) {
      const auto canonical_root = canonical(root);

      create_directories(root / "moo");
      create_directories(root / "foo");
      create_directory_symlink(root / "moo", root / "foo/abc");

      REQUIRE(u8path("moo/bar/file.txt")
              == weakly_canonical(root / "foo/abc/bar/file.txt")
              .lexically_relative(canonical_root));

      // the return value is a path in normal form
      REQUIRE(u8path("moo\\bar\\gaz\\.")
              == weakly_canonical(root / "foo/abc/./bar//gaz/.")
                   .lexically_relative(canonical_root));
    });
    });
#else
  with_temp_dir([&](const path& root) {
    const auto canonical_root = canonical(root);

    create_directories(root / "moo");
    create_directories(root / "foo");
    create_directory_symlink(root / "moo", root / "foo/abc");

    REQUIRE(u8path("moo/file.txt")
            == weakly_canonical(root / "foo/abc/bar/../file.txt")
                 .lexically_relative(canonical_root));

    // the return value is a path in normal form
    REQUIRE(u8path("gaz/.")
            == weakly_canonical(root / "foo/abc/./bar//../../gaz/.")
                 .lexically_relative(canonical_root));
  });
#endif
}


TEST_CASE("weakly_canonical with (broken) symlinks", "[operations]")
{
#if defined(FSPP_IS_WIN)
  with_privilege_check([]() {
    with_temp_dir([&](const path& root) {
      const auto canonical_root = canonical(root);

      create_directories(root / "foo");
      create_directory_symlink("bar", root / "foo/abc");

#if 0
      REQUIRE(u8path("foo/file.txt")
              == weakly_canonical(root / "foo/abc/file.txt")
                   .lexically_relative(canonical_root));
#endif
      // the return value is a path in normal form
      REQUIRE(u8path("foo/abc/moo/gaz/.")
              == weakly_canonical(root / "foo/abc/./moo//gaz/.")
                   .lexically_relative(canonical_root));
    });
  });
#else
  with_temp_dir([&](const path& root) {
    const auto canonical_root = canonical(root);

    create_directories(root / "foo");
    create_directory_symlink("./bar", root / "foo/abc");

    REQUIRE(u8path("foo/file.txt")
            == weakly_canonical(root / "foo/abc/../file.txt")
                 .lexically_relative(canonical_root));

    // the return value is a path in normal form
    REQUIRE(u8path("foo/gaz/.")
            == weakly_canonical(root / "foo/abc/./moo//../../gaz/.")
                 .lexically_relative(canonical_root));
  });
#endif
}


TEST_CASE("weakly_canonical with entire missing root", "[operations]")
{
  REQUIRE(current_path() / "very-unlikely-to-exist/foo/file.txt"
          == weakly_canonical(current_path() / "very-unlikely-to-exist/foo/bar/../file.txt"));
}


}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
