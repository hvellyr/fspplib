// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/utility/scope.hpp"

#include <chrono>
#include <iostream>
#include <ostream>


namespace eyestep {
namespace utility {

inline Scope
make_timer_logger(const std::string& msg, std::ostream& os = std::clog)
{
  const auto start = std::chrono::system_clock::now();
  return make_scope([&os, msg, start]() {
    const auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    os << "Diff for " << msg << ": " << diff.count() * 1000.0 << " ms" << std::endl;
  });
}

}  // namespace utility
}  // namespace eyestep
