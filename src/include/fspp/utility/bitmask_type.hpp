// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include <cstdint>


#define FSPP_BITMASK_TYPE(_type_)                                                        \
  inline _type_ operator&(_type_ a, _type_ b)                                            \
  {                                                                                      \
    return static_cast<_type_>(static_cast<uint_least32_t>(a)                            \
                               & static_cast<uint_least32_t>(b));                        \
  }                                                                                      \
                                                                                         \
  inline _type_ operator|(_type_ a, _type_ b)                                            \
  {                                                                                      \
    return static_cast<_type_>(static_cast<uint_least32_t>(a)                            \
                               | static_cast<uint_least32_t>(b));                        \
  }                                                                                      \
                                                                                         \
  inline _type_ operator^(_type_ a, _type_ b)                                            \
  {                                                                                      \
    return static_cast<_type_>(static_cast<uint_least32_t>(a)                            \
                               ^ static_cast<uint_least32_t>(b));                        \
  }                                                                                      \
                                                                                         \
  inline _type_ operator~(_type_ a)                                                      \
  {                                                                                      \
    return static_cast<_type_>(~static_cast<uint_least32_t>(a));                         \
  }                                                                                      \
                                                                                         \
  inline _type_& operator&=(_type_& a, _type_ b)                                         \
  {                                                                                      \
    a = a & b;                                                                           \
    return a;                                                                            \
  }                                                                                      \
                                                                                         \
  inline _type_& operator|=(_type_& a, _type_ b)                                         \
  {                                                                                      \
    a = a | b;                                                                           \
    return a;                                                                            \
  }                                                                                      \
                                                                                         \
  inline _type_& operator^=(_type_& a, _type_ b)                                         \
  {                                                                                      \
    a = a ^ b;                                                                           \
    return a;                                                                            \
  }                                                                                      \
                                                                                         \
  inline bool operator==(_type_ a, int b) { return static_cast<int>(a) == b; }           \
                                                                                         \
  inline bool operator!=(_type_ a, int b) { return !(a == b); }
