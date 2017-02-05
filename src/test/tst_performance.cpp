// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file_status.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/utility/time_logger.hpp"
#include "fspp/utils.hpp"

#include "test_utils.hpp"

#include <catch/catch.hpp>

#include <algorithm>
#include <ostream>
#include <random>


namespace eyestep {
namespace filesystem {
namespace tests {

namespace {
void
create_level(const path& base_p, int fcount, int dcount, int depth_count)
{
  static int d_count = 0;
  static int f_count = 0;

  for (auto i = 0; i < fcount; ++i) {
    const auto f_p = (base_p / "f-").concat(std::to_string(++f_count));
    write_file(f_p, make_random_string(32));
  }

  for (auto i = 0; i < dcount; ++i) {
    const auto dir_p = (base_p / "d-").concat(std::to_string(++d_count));
    create_directory(dir_p);
    if (depth_count > 0) {
      create_level(dir_p, fcount, dcount, depth_count - 1);
    }
  }
}
}  // anon namespace


TEST_CASE("copy - directory recursive large", "[.][performance]")
{
  //  auto root = create_temp_dir(temp_directory_path());
  with_temp_dir([](const path& root) {
    auto src_p = root / "src";
    create_directories(src_p);

    {
      auto time_guard = utility::make_timer_logger("create_test_tree", std::cout);
      create_level(src_p, 10, 2, 10);
    }

    {
      auto time_guard = utility::make_timer_logger("copy", std::cout);
      copy(src_p, root / "dst", copy_options::recursive);
    }

    REQUIRE(!is_empty(root / "dst"));
  });
}

}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
