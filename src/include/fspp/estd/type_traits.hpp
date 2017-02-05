// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include <type_traits>


namespace eyestep {
namespace estd {

template <bool Predicate, typename T = void>
using enable_if_t = typename std::enable_if<Predicate, T>::type;

}  // namespace estd
}  // namespace eyestep
