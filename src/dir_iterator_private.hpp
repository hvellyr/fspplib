// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/dir_iterator.hpp"

#include <memory>
#include <system_error>

namespace eyestep {
namespace filesystem {

class directory_iterator::IDirIterImpl
{
public:
  virtual ~IDirIterImpl() {}

  virtual void increment(std::error_code& ec) = 0;
  virtual const directory_entry& object() const = 0;
  virtual bool equal(const IDirIterImpl* other) const = 0;
  virtual bool is_end() const = 0;

protected:
  IDirIterImpl() = default;
  IDirIterImpl(const IDirIterImpl&) = default;
};


namespace impl {

std::unique_ptr<directory_iterator::IDirIterImpl>
make_dir_iterator(const path& p, std::error_code& ec);

bool
is_access_error(const std::error_code& ec);

}  // namespace impl

}  // namespace filesystem
}  // namespace eyestep
