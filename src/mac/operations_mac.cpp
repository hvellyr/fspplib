// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/operations.hpp"

#include "operations_impl.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/types.hpp"
#include "fspp/estd/memory.hpp"

#include <dirent.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <utime.h>

#include <array>
#include <system_error>


namespace eyestep {
namespace filesystem {

path
temp_directory_path(std::error_code& ec) NOEXCEPT
{
  char tmp[PATH_MAX];
  if (::confstr(_CS_DARWIN_USER_TEMP_DIR, tmp, PATH_MAX) == 0) {
    ec = std::error_code(errno, std::generic_category());
    return {};
  }

  ec.clear();
  return u8path(tmp);
}

}  // namespace filesystem
}  // namespace eyestep
