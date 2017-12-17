// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file_status.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"
#include "fspp/utility/scope.hpp"
#include "fspp/utils.hpp"

#include "test_utils.hpp"

#include <catch/catch.hpp>

#include <algorithm>
#include <ostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if defined(FSPP_IS_MAC)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


namespace eyestep {
namespace filesystem {

static std::ostream&
operator<<(std::ostream& os, recursive_directory_iterator const& val)
{
  os << "[recursive_directory_iterator:";
  if (val == recursive_directory_iterator()) {
    os << "end";
  }
  else {
    os << *val;
  }
  os << "]";
  return os;
}


namespace tests {

TEST_CASE("basic", "[dir-iter]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "abc/others");

    write_file(root / "abc/german.txt", "Hallo Welt!");
    write_file(root / "abc/kor.txt", "annyeong sesang");
    write_file(root / "abc/en.txt", "hello world");

    std::set<path> found;
    for (const auto& entry : directory_iterator(root / "abc")) {
      found.insert(entry.path().filename());
    }

    REQUIRE(found == (std::set<path>{"german.txt", "kor.txt", "en.txt", "others"}));
  });
}


TEST_CASE("empty folder", "[dir-iter]")
{
  with_temp_dir([](const path& root) {
    create_directories(root / "abc");

    std::set<path> found;
    for (const auto& entry : directory_iterator(root / "abc")) {
      found.insert(entry.path().filename());
    }

    REQUIRE(found.empty());
  });
}


namespace {
void
test_directory_setup(const path& root)
{
  create_directories(root / "abc/foo");
  create_directories(root / "xyz/a/b");

  write_file(root / "german.txt", "Hallo Welt");
  write_file(root / "kor.txt", "annyeong sesang");

  write_file(root / "abc/en.txt", "hello world");
  write_file(root / "abc/foo/fr.txt", "Bonjour le monde");
  write_file(root / "abc/foo/es.txt", "Hola Mundo");
}


PathDepthPairs&
sort_depths(PathDepthPairs& depths)
{
  using std::begin;
  using std::end;

  std::sort(
    begin(depths), end(depths),
    [](const PathDepthPair& a, const PathDepthPair& b) { return a.first < b.first; });

  return depths;
}

}  // anon namespace


TEST_CASE("recursive dir iter - empty folder", "[dir-iter][recursive]")
{
  with_temp_dir([](const path& root) {
    auto iter = recursive_directory_iterator(root);
    REQUIRE(iter == recursive_directory_iterator());
  });
}


TEST_CASE("recursive dir iter", "[dir-iter][recursive]")
{
  with_temp_dir([](const path& root) {
    test_directory_setup(root);

    PathDepthPairs depths;

    auto iter = recursive_directory_iterator(root);
    for (; iter != end(iter); ++iter) {
      depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
    }

    REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                     {"abc", 0},
                                     {"abc/en.txt", 1},
                                     {"abc/foo", 1},
                                     {"abc/foo/es.txt", 2},
                                     {"abc/foo/fr.txt", 2},
                                     {"german.txt", 0},
                                     {"kor.txt", 0},
                                     {"xyz", 0},
                                     {"xyz/a", 1},
                                     {"xyz/a/b", 2},
                                   }));
  });
}


TEST_CASE("recursive dir iter - skipping", "[dir-iter][recursive]")
{
  with_temp_dir([](const path& root) {
    test_directory_setup(root);

    PathDepthPairs depths;

    auto iter = recursive_directory_iterator(root);
    for (; iter != end(iter); ++iter) {
      depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
      if (iter->path().filename() == "kor.txt") {
        // disabling recursion on a file, doesn't have an effect on following
        // directories.
        iter.disable_recursion_pending();
      }
      else if (iter->path().filename() == "foo") {
        iter.disable_recursion_pending();
      }
    }

    REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                     {"abc", 0},
                                     {"abc/en.txt", 1},
                                     {"abc/foo", 1},
                                     {"german.txt", 0},
                                     {"kor.txt", 0},
                                     {"xyz", 0},
                                     {"xyz/a", 1},
                                     {"xyz/a/b", 2},
                                   }));
  });
}


TEST_CASE("recursive dir iter - don't follow dir-symlinks", "[dir-iter][recursive]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      test_directory_setup(root);
      create_directory_symlink(root / "abc/foo", root / "eskimo");

      PathDepthPairs depths;

      auto iter = recursive_directory_iterator(root, directory_options::none);
      for (; iter != end(iter); ++iter) {
        depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
      }

      REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                       {"abc", 0},
                                       {"abc/en.txt", 1},
                                       {"abc/foo", 1},
                                       {"abc/foo/es.txt", 2},
                                       {"abc/foo/fr.txt", 2},
                                       {"eskimo", 0},
                                       {"german.txt", 0},
                                       {"kor.txt", 0},
                                       {"xyz", 0},
                                       {"xyz/a", 1},
                                       {"xyz/a/b", 2},
                                     }));
    });
  });
}


TEST_CASE("recursive dir iter - follow dir-symlinks", "[dir-iter][recursive]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      test_directory_setup(root);
      create_directory_symlink(root / "abc/foo", root / "eskimo");

      PathDepthPairs depths;

      for (auto iter = recursive_directory_iterator(
             root, directory_options::follow_directory_symlink);
           iter != end(iter); ++iter) {
        depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
      }

      REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                       {"abc", 0},
                                       {"abc/en.txt", 1},
                                       {"abc/foo", 1},
                                       {"abc/foo/es.txt", 2},
                                       {"abc/foo/fr.txt", 2},
                                       {"eskimo", 0},
                                       {"eskimo/es.txt", 1},
                                       {"eskimo/fr.txt", 1},
                                       {"german.txt", 0},
                                       {"kor.txt", 0},
                                       {"xyz", 0},
                                       {"xyz/a", 1},
                                       {"xyz/a/b", 2},
                                     }));
    });
  });
}


TEST_CASE("recursive dir iter - file-symlinks", "[dir-iter][recursive]")
{
  with_privilege_check([]() {
    with_temp_dir([](const path& root) {
      test_directory_setup(root);
      create_symlink(root / "abc/foo/fr.txt", root / "fr-CA.txt");

      PathDepthPairs depths;

      for (auto iter = recursive_directory_iterator(
             root, directory_options::follow_directory_symlink);
           iter != end(iter); ++iter) {
        depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
      }

      REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                       {"abc", 0},
                                       {"abc/en.txt", 1},
                                       {"abc/foo", 1},
                                       {"abc/foo/es.txt", 2},
                                       {"abc/foo/fr.txt", 2},
                                       {"fr-CA.txt", 0},
                                       {"german.txt", 0},
                                       {"kor.txt", 0},
                                       {"xyz", 0},
                                       {"xyz/a", 1},
                                       {"xyz/a/b", 2},
                                     }));
    });
  });
}


// permissions are not easy to test on windows.  This test is disabled
// for now.
#if !defined(FSPP_IS_WIN)
TEST_CASE("recursive dir iter - skip-permissions", "[dir-iter][recursive]")
{
  with_temp_dir([](const path& root) {
    try {
      test_directory_setup(root);
      permissions(root / "abc", perms::others_read);

      PathDepthPairs depths;

      for (auto iter = recursive_directory_iterator(
             root, directory_options::skip_permission_denied);
           iter != end(iter); ++iter) {
        depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
      }

      REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                       {"abc", 0},
                                       {"german.txt", 0},
                                       {"kor.txt", 0},
                                       {"xyz", 0},
                                       {"xyz/a", 1},
                                       {"xyz/a/b", 2},
                                     }));
      permissions(root / "abc", perms::owner_all);
    }
    catch (...) {
      permissions(root / "abc", perms::owner_all);
    }
  });
}
#endif


TEST_CASE("recursive dir iter - pop", "[dir-iter][recursive]")
{
  with_temp_dir([](const path& root) {
    test_directory_setup(root);
    create_directory(root / "abc/xaxa");

    PathDepthPairs depths;

    auto iter = recursive_directory_iterator(root);
    while (iter != end(iter)) {
      if (iter->path().parent_path().filename() == "foo") {
        iter.pop();
      }
      else {
        depths.emplace_back(iter->path().lexically_relative(root), iter.depth());
        ++iter;
      }
    }

    REQUIRE(sort_depths(depths) == (PathDepthPairs{
                                     {"abc", 0},
                                     {"abc/en.txt", 1},
                                     {"abc/foo", 1},
                                     {"abc/xaxa", 1},
                                     {"german.txt", 0},
                                     {"kor.txt", 0},
                                     {"xyz", 0},
                                     {"xyz/a", 1},
                                     {"xyz/a/b", 2},
                                   }));
  });
}

}  // namespace tests
}  // namespace filesystem
}  // namespace eyestep
