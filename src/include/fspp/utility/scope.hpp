// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/platform.hpp"

#include <functional>
#include <utility>


namespace eyestep {
namespace utility {

/*! Scope guard which invokes a function on scope exit
 *
 * @code
 * void foo() {
 *   int val = 42;
 *   {
 *     Scope guard([&val]() { val = 11; });
 *     //...
 *   }
 *   // val => 11
 * }
 * @endcode
 */
class Scope
{
public:
  /*! Creates a new scope with a given function
   *
   * The function is kept and invoked when the Scope's destructor is run.  Note that @p
   * atExitCallback *must not* throw an exception. */
  explicit Scope(std::function<void() NOEXCEPT> atExitCallback)
    : mAtExitCallback(std::move(atExitCallback))
  {
  }

  ~Scope()
  {
    if (mAtExitCallback) {
      mAtExitCallback();
    }
  }

  /*! Default ctor which creates a noop scope guard */
  Scope() = default;

  Scope(Scope&& other)
    : mAtExitCallback(std::move(other.mAtExitCallback))
  {
    other.mAtExitCallback = nullptr;
  }

  auto operator=(Scope&& other) -> Scope&
  {
    if (mAtExitCallback) {
      mAtExitCallback();
    }

    mAtExitCallback = std::move(other.mAtExitCallback);
    other.mAtExitCallback = nullptr;
    return *this;
  }

  Scope(const Scope&) = delete;
  auto operator=(const Scope&) -> Scope& = delete;

private:
  std::function<void() NOEXCEPT> mAtExitCallback;
};


/*! Creates a scope guard invoking a function on scope exit
 *
 * @code
 * void foo() {
 *   int val = 42;
 *   {
 *     auto guard = makeScope([&val]() { val = 11; });
 *     //...
 *   }
 *   // val => 11
 * }
 * @endcode
 */
template <typename F>
auto
make_scope(F functor) -> Scope
{
  return Scope(functor);
}


/*! Creates a value scope guard which saves @p variable's value and immediately sets it to
 * @p newValue.  The guard will restore @p variable's value on scope exit.
 *
 * @code
 * int foo = 42;
 * {
 *   auto guard = makeValueScope(foo, 21);
 *   // foo -> 21
 * }
 * // foo -> 42
 * @endcode
 */
template <typename T>
auto
make_value_scope(T& variable, T newValue) -> Scope
{
  using std::swap;
  swap(variable, newValue);

  return scope([&variable, newValue] { variable = newValue; });
}


/*! Creates a value scope guard which saves @p variable's value and restores it on scope
 *  exit.
 *
 * @code
 * int foo = 42;
 * {
 *   auto guard = makeValueScope(foo);
 *   // foo -> 42
 *   foo = 11;
 *   // ...
 * }
 * // foo -> 42
 * @endcode
 */
template <typename T>
auto
make_value_scope(T& variable) -> Scope
{
  return make_value_scope(variable, variable);
}

}  // namespace utility
}  // namespace eyestep
