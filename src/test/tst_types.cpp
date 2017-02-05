// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file_status.hpp"
#include "fspp/details/types.hpp"

#include <catch/catch.hpp>


namespace eyestep {
namespace tests {

TEST_CASE("Basic permissions", "[types]")
{
  namespace fs = eyestep::filesystem;

  auto p644 = (fs::perms::owner_all | fs::perms::group_read | fs::perms::others_read);

  REQUIRE((p644 & fs::perms::owner_read) == fs::perms::owner_read);
  REQUIRE((p644 & fs::perms::group_write) == 0);
}


TEST_CASE("Basic copy_options", "[types]")
{
  namespace fs = eyestep::filesystem;

  auto ops = fs::copy_options::skip_existing | fs::copy_options::create_hard_links;
  REQUIRE((ops & fs::copy_options::skip_existing) == fs::copy_options::skip_existing);
  REQUIRE((ops & fs::copy_options::create_hard_links)
          == fs::copy_options::create_hard_links);
  REQUIRE((ops & fs::copy_options::copy_symlinks) == 0);
}


TEST_CASE("File status", "[types]")
{
  namespace fs = eyestep::filesystem;

  SECTION("default ctor")
  {
    auto f = fs::file_status();
    REQUIRE(f.type() == fs::file_type::none);
    REQUIRE(f.permissions() == fs::perms::unknown);
  }

  SECTION("assign operator")
  {
    auto f = fs::file_status(fs::file_type::regular, fs::perms::owner_exec);
    auto s = std::move(f);

    REQUIRE(s.type() == fs::file_type::regular);
    REQUIRE(s.permissions() == fs::perms::owner_exec);
  }
}
}  // namespace tests
}  // namespace eyestep
