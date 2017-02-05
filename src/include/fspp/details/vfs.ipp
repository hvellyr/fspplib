// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/utility/scope.hpp"

#include <string>
#include <utility>


namespace eyestep {
namespace filesystem {
namespace vfs {

template <typename Functor>
void
with_memory_vfs(const std::string& name, Functor functor)
{
  auto fs = make_memory_filesystem();
  auto& fs_ref = *fs;
  register_vfs(name, std::move(fs));
  auto vfs_guard = utility::make_scope([&]() { unregister_vfs(name); });
  functor(fs_ref);
}

}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep
