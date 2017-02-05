// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#if defined(FSPP_HAVE_STD_OPTIONAL)
#include <optional>
#endif

#include <utility>


namespace eyestep {
namespace estd {

#if !defined(FSPP_HAVE_STD_OPTIONAL)

template <typename T>
class optional
{
public:
  optional() = default;
  optional(T val)
    : _value(std::move(val))
    , _is_set(true)
  {
  }

  optional(const optional<T>& rhs) = default;

#if defined(FSPP_IS_VS2013)
  optional(optional<T>&& rhs)
    : _value(std::move(rhs._value))
    , _is_set(rhs._is_set)
  {
  }
#else
  optional(optional<T>&&) = default;
#endif

  optional<T>& operator=(const optional<T>& rhs) = default;

#if defined(FSPP_IS_VS2013)
  optional<T>& operator=(optional<T>&& rhs)
  {
    _value = std::move(rhs._value);
    _is_set = rhs._is_set;
    return *this;
  }
#else
  optional<T>& operator=(optional<T>&&) = default;
#endif

  explicit operator bool() const { return _is_set; }

  const T& value() const { return _value; }
  T& value() { return _value; }

  void reset()
  {
    if (_is_set) {
      _is_set = false;
      _value.T::~T();
    }
  }

private:
  T _value;
  bool _is_set = false;
};

#else

using std::optional;

#endif

}  // namespace estd
}  // namespace eyestep
