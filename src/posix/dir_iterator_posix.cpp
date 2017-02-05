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
class PosixDirIterImpl : public directory_iterator::IDirIterImpl
{
public:
  PosixDirIterImpl(DIR* dirp, path p)
    : _dirp(dirp)
    , _path(std::move(p))
  {
  }

  ~PosixDirIterImpl()
  {
    if (_dirp) {
      ::closedir(_dirp);
    }
  }

  void increment(std::error_code& ec) override
  {
    struct dirent* direntp = nullptr;
    struct dirent f;

    do {
      if (::readdir_r(_dirp, &f, &direntp)) {
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

    ::closedir(_dirp);
    _dirp = nullptr;

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

  auto impl = estd::make_unique<PosixDirIterImpl>(dirp, p);
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
