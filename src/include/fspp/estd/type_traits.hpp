// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

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
