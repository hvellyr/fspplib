// Copyright (c) 2016 Gregor Klinke

#pragma once

#include <istream>
#include <system_error>


namespace eyestep {
namespace filesystem {

namespace detail {
/*! Interface file backends have to implement.
 *
 * @attention Concrete implementation should provide their own destructor closing the
 *            underlying stream and releasing resources.
*/
class IFileImpl
{
public:
  virtual ~IFileImpl() = default;

  virtual std::iostream& open(const path& path,
                              std::ios::openmode mode,
                              std::error_code& ec) = 0;
  virtual bool is_open() const = 0;
  virtual void close(std::error_code& ec) = 0;
};

}  // namespace detail

}  // namespace filesystem
}  // namespace eyestep
