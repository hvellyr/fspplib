// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/path.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/estd/optional.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>


namespace eyestep {
namespace filesystem {
namespace vfs {

bool
is_vfs_root_name(const path::string_type& rootname) NOEXCEPT;
IFilesystem*
find_vfs(const path::string_type& rootname) NOEXCEPT;
path
deroot(const path& src) NOEXCEPT;

template <typename T, typename Functor>
estd::optional<T>
with_vfs_do(const path& p, Functor functor) NOEXCEPT
{
  auto rootname = p.root_name().native();
  if (is_vfs_root_name(rootname)) {
    if (auto* fs = find_vfs(rootname)) {
      return {functor(*fs, deroot(p))};
    }
    else {
      return {T()};
    }
  }

  return {};
}


template <typename T, typename Functor>
estd::optional<T>
with_vfs_do(const path& p1, const path& p2, Functor functor) NOEXCEPT
{
  auto rootname = p1.root_name().native();
  if (is_vfs_root_name(rootname)) {
    if (auto* fs = find_vfs(rootname)) {
      assert(p1.root_name() == p2.root_name());
      return {functor(*fs, deroot(p1), deroot(p2))};
    }
    else {
      return {T()};
    }
  }

  return {};
}

}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep
