// Copyright (c) 2016 Gregor Klinke

#include "fspp/limits.hpp"

#include <limits.h>
#include <unistd.h>


namespace eyestep {
namespace filesystem {

uintmax_t
symlink_loop_maximum() NOEXCEPT
{
  return static_cast<uintmax_t>(sysconf(_SC_SYMLOOP_MAX));
}

}  // namespace filesystem
}  // namespace eyestep
