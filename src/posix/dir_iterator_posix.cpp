// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/dir_iterator.hpp"

#include "dir_iterator_private.hpp"

#include "fspp/estd/memory.hpp"

#include <cstring>
#include <dirent.h>

#include <system_error>


namespace eyestep {
namespace filesystem {

namespace {
#if defined(FSPP_USE_READDIR_R)
size_t dirent_buf_size(DIR* dirp, std::error_code& ec)
{
  long name_max;
#if defined(FSPP_HAVE_FPATHCONF) && defined(FSPP_HAVE_DIRFD)   \
    && defined(_PC_NAME_MAX)
  name_max = fpathconf(dirfd(dirp), _PC_NAME_MAX);
  if (name_max == -1)
#  if defined(NAME_MAX)
    name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
#  else
  ec = std::make_error_code(std::errc::not_supported);
  return (size_t)(-1);
# endif
# elif defined(NAME_MAX)
  name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
# else
#   error "buffer size for readdir_r cannot be determined"
# endif

  ec.clear();
  auto name_end = (size_t)offsetof(struct dirent, d_name) + name_max + 1;
  return name_end > sizeof(struct dirent)
    ? name_end
    : sizeof(struct dirent);
}
#endif


class PosixDirIterImpl : public directory_iterator::IDirIterImpl
{
public:
#if defined(FSPP_USE_READDIR_R)
  PosixDirIterImpl(DIR* dirp, path p, size_t dirent_size = 0)
    : _dirp(dirp)
    , _path(std::move(p))
    , _dirent_size(dirent_size)
  {
  }
#else
  PosixDirIterImpl(DIR* dirp, path p)
    : _dirp(dirp)
    , _path(std::move(p))
  {
  }
#endif

  void close_dir()
  {
#if defined(FSPP_USE_READDIR_R)
    if (_dirent) {
      ::free(_dirent);
      _dirent = nullptr;
    }
#endif
    if (_dirp) {
      ::closedir(_dirp);
      _dirp = nullptr;
    }
  }

  ~PosixDirIterImpl() override
  {
    close_dir();
  }

  void increment(std::error_code& ec) override
  {
#if defined(FSPP_USE_READDIR_R)
    struct dirent* direntp = nullptr;

    if (!_dirent) {
      _dirent = reinterpret_cast<struct dirent *>(::malloc(_dirent_size));
    }

    do {
      if (::readdir_r(_dirp, _dirent, &direntp)) {
        ec = std::error_code(errno, std::generic_category());
        return;
      }

      if (direntp) {
        if (::strcmp(direntp->d_name, "..") == 0 || ::strcmp(direntp->d_name, ".") == 0) {
          continue;
        }

        _current.assign(_path / direntp->d_name);
        ec.clear();
        return;
      }
    } while (direntp);
#else
    struct dirent* direntp = nullptr;

    do {
      errno = 0;
      direntp = ::readdir(_dirp);
      if (direntp) {
        if (::strcmp(direntp->d_name, "..") == 0 || ::strcmp(direntp->d_name, ".") == 0) {
          continue;
        }

        _current.assign(_path / direntp->d_name);
        ec.clear();
        return;
      }
      else if (errno != 0) {
        ec = std::error_code(errno, std::generic_category());
        return;
      }
    } while (direntp);
#endif

    close_dir();
    ec.clear();
  }

  const directory_entry& object() const override { return _current; }

  bool equal(const IDirIterImpl* other) const override
  {
    if (auto mac_impl = dynamic_cast<const PosixDirIterImpl*>(other)) {
      if (is_end() == mac_impl->is_end()) {
        if (!is_end()) {
          return _current == mac_impl->_current;
        }

        return true;
      }
    }

    return false;
  }

  bool is_end() const override { return !_dirp; }

private:
  DIR* _dirp = nullptr;
  path _path;
#if defined(FSPP_USE_READDIR_R)
  struct dirent* _dirent = nullptr;
  size_t _dirent_size = 0;
#endif
  directory_entry _current;
};

}  // anon namespace


std::unique_ptr<directory_iterator::IDirIterImpl>
impl::make_dir_iterator(const path& p, std::error_code& ec)
{
  auto dirp = ::opendir(p.c_str());
  if (!dirp) {
    ec = std::error_code(errno, std::generic_category());
    return nullptr;
  }

#if defined(FSPP_USE_READDIR_R)
  const auto dirent_size = dirent_buf_size(dirp, ec);
  if (ec) {
    return {};
  }
  auto impl = estd::make_unique<PosixDirIterImpl>(dirp, p, dirent_size);
#else
  auto impl = estd::make_unique<PosixDirIterImpl>(dirp, p);
#endif

  impl->increment(ec);
  if (ec || impl->is_end()) {
    return {};
  }
  return std::move(impl);
}


bool
impl::is_access_error(const std::error_code& ec)
{
  return ec.category() == std::generic_category() && ec.value() == EACCES;
}

}  // namespace filesystem
}  // namespace eyestep
