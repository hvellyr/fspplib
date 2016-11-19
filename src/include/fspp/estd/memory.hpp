// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/estd/type_traits.hpp"

#include <memory>
#include <type_traits>
#include <utility>


namespace eyestep {
namespace estd {

template <class T, class... Args>
auto
make_unique(Args&&... args) -> enable_if_t<!std::is_array<T>::value, std::unique_ptr<T>>
{
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template <class T>
auto
make_unique(std::size_t size) -> enable_if_t<std::is_array<T>::value, std::unique_ptr<T>>
{
  return std::unique_ptr<T>{new typename std::remove_extent<T>::type[size]()};
}

}  // namespace estd
}  // namespace eyestep
