// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include <utility>


namespace eyestep {
namespace estd {

/*! A C++14 compliant std::mismatch() variant.  */
template <class InputIt1, class InputIt2, class Predicate>
std::pair<InputIt1, InputIt2>
mismatch(
  InputIt1 i_first1, InputIt1 i_last1, InputIt2 i_first2, InputIt2 i_last2, Predicate p)
{
  while (i_first1 != i_last1 && i_first2 != i_last2 && p(*i_first1, *i_first2)) {
    ++i_first1;
    ++i_first2;
  }
  return std::make_pair(i_first1, i_first2);
}

}  // namespace estd
}  // namespace eyestep
