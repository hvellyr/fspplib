// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/operations.hpp"

#include "operations_impl.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/types.hpp"

#include <unistd.h>

#include <system_error>


namespace eyestep {
namespace filesystem {

path
temp_directory_path(std::error_code& ec) NOEXCEPT
{
  ec.clear();

  if (auto value = ::getenv("TMPDIR")) {
    return u8path(value);
  }

  return u8path("/tmp");
}

}  // namespace filesystem
}  // namespace eyestep
