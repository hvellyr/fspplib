// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include <type_traits>


namespace eyestep {
namespace estd {

#if !defined(FSPP_HAVE_STD_ENABLE_IF_T)

template <bool Predicate, typename T = void>
using enable_if_t = typename std::enable_if<Predicate, T>::type;

#else

using std::enable_if_t;

#endif

}  // namespace estd
}  // namespace eyestep
