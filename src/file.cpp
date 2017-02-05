// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/file.hpp"

#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/path.hpp"

#include "vfs_private.hpp"

#include <cassert>
#include <ctime>
#include <fstream>
#include <memory>
#include <system_error>
#include <utility>


namespace eyestep {
namespace fs = filesystem;

namespace filesystem {

class RealFileImpl : public detail::IFileImpl
{
  bool _is_open = false;
  std::fstream _stream;

public:
  ~RealFileImpl() override
  {
    if (is_open()) {
      std::error_code ec;
      close(ec);
    }
  }

  std::iostream& open(const path& p,
                      std::ios::openmode mode,
                      std::error_code& ec) override
  {
    assert(!is_open());

    _stream.open(p.c_str(), mode);
    if (_stream.fail()) {
      ec = std::make_error_code(std::errc::io_error);
    }
    else {
      _is_open = true;
      ec.clear();
    }

    return _stream;
  }

  bool is_open() const override { return _is_open; }

  void close(std::error_code& ec) override
  {
    if (_is_open) {
      // clear any error bit from the stream before trying to close.
      // Otherwise the following check will detect the wrong bits.
      _stream.clear();
      _stream.close();
      if (_stream.fail()) {
        ec = std::make_error_code(std::errc::io_error);
      }
      else {
        ec.clear();
      }

      _is_open = false;
    }
    else {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
    }
  }
};


//----------------------------------------------------------------------------------------

File::File(fs::path p)
  : _path(std::move(p))
{
  if (auto val = vfs::with_vfs_do<std::unique_ptr<detail::IFileImpl>>(
        _path,
        [](vfs::IFilesystem& fs, const fs::path&) { return fs.make_file_impl(); })) {
    _impl = std::shared_ptr<detail::IFileImpl>(std::move(val.value()));
  }
  else {
    _impl = std::make_shared<RealFileImpl>();
  }
}


File::File(File&& other)
  : _impl(std::move(other._impl))
  , _path(std::move(other._path))
{
}


File&
File::operator=(File&& other)
{
  _impl = std::move(other._impl);
  _path = std::move(other._path);
  return *this;
}


const path&
File::path() const
{
  return _path;
}


bool
File::is_valid() const
{
  return _impl != nullptr;
}


std::iostream&
File::open(std::ios::openmode mode, std::error_code& ec)
{
  return _impl->open(_path, mode, ec);
}


std::iostream&
File::open(std::ios::openmode mode)
{
  std::error_code ec;
  auto& stream = _impl->open(_path, mode, ec);
  if (ec) {
    throw filesystem_error("Failed opening file", _path, ec);
  }

  return stream;
}


bool
File::is_open() const
{
  return _impl->is_open();
}


void
File::close(std::error_code& ec)
{
  _impl->close(ec);
}


void
File::close()
{
  std::error_code ec;
  _impl->close(ec);
  if (ec) {
    throw filesystem_error("Failed closing file", _path, ec);
  }
}

}  // namespace filesystem
}  // namespace eyestep
