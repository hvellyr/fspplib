// Copyright (c) 2016 Gregor Klinke

#include "fspp/limits.hpp"

namespace eyestep {
namespace filesystem {

uintmax_t
symlink_loop_maximum() NOEXCEPT
{
  return 31;
}

}  // namespace filesystem
}  // namespace eyestep
