// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/platform.hpp"

#include <cstdint>


namespace eyestep {
namespace filesystem {

/*! Maximum number of symbolic links that can be reliably traversed in the
 *  resolution of a pathname in the absence of a loop. */
FSPP_API uintmax_t
symlink_loop_maximum() NOEXCEPT;

}  // namespace filesystem
}  // namespace eyestep
