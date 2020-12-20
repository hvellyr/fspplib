// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

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
  optional()
    : _is_set(false)
  {
  }

  optional(T val)
    : _is_set(true)
  {
    new (&_value) T(std::move(val));
  }

  optional(const optional<T>& rhs)
    : _is_set(rhs._is_set)
  {
    if (rhs._is_set) {
      new (&_value) T(rhs._value);
    }
  }

  optional(optional<T>&& rhs)
    : _is_set(rhs._is_set)
  {
    if (rhs._is_set) {
      new (&_value) T(std::move(rhs._value));
    }
  }

  optional<T>& operator=(const optional<T>& rhs)
  {
    reset();

    _is_set = rhs._is_set;
    if (rhs._is_set) {
      new (&_value) T(rhs._value);
    }

    return *this;
  }

  optional<T>& operator=(optional<T>&& rhs)
  {
    reset();

    _is_set = rhs._is_set;
    if (rhs._is_set) {
      new (&_value) T(std::move(rhs._value));
    }

    return *this;
  }

  optional<T>& operator=(const T& val)
  {
    reset();
    _is_set = true;
    new (&_value) T(val);
    return *this;
  }

  optional<T>& operator=(T&& val)
  {
    reset();
    _is_set = true;
    new (&_value) T(std::move(val));
    return *this;
  }

  ~optional() { reset(); }

  bool has_value() const { return _is_set; }
  explicit operator bool() const { return _is_set; }

  const T& value() const { return _value; }
  T& value() { return _value; }

  const T* operator->() const { return &_value; }
  T* operator->() { return &_value; }
  const T& operator*() const { return _value; }
  T& operator*() { return _value; }

  void reset()
  {
    if (_is_set) {
      _is_set = false;
      _value.T::~T();
    }
  }

private:
  union
  {
    bool _undefined;
    T _value;
  };
  bool _is_set = false;
};

#else

using std::optional;

#endif

}  // namespace estd
}  // namespace eyestep
