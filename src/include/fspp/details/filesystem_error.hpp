// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/path.hpp"
#include "fspp/details/platform.hpp"

#include <string>
#include <system_error>


namespace eyestep {
namespace filesystem {

class filesystem_error : public std::system_error
{
public:
  filesystem_error(const std::string& what_arg, std::error_code ec)
    : system_error(ec, what_arg)
  {
  }

  filesystem_error(const std::string& what_arg, const path& p1, std::error_code ec)
    : system_error(ec, what_arg)
    , _path1(p1)
  {
  }

  filesystem_error(const std::string& what_arg,
                   const path& p1,
                   const path& p2,
                   std::error_code ec)
    : system_error(ec, what_arg)
    , _path1(p1)
    , _path2(p2)
  {
  }

  const path& path1() const NOEXCEPT { return _path1; }
  const path& path2() const NOEXCEPT { return _path2; }

private:
  path _path1;
  path _path2;
};

}  // namespace filesystem
}  // namespace eyestep
