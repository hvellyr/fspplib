// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/dir_iterator.hpp"

#include "dir_iterator_private.hpp"

#include "fspp/estd/memory.hpp"

#include <windows.h>


namespace eyestep {
namespace filesystem {

namespace {
class WinDirIterImpl : public directory_iterator::IDirIterImpl
{
  enum class State
  {
    init,
    cont,
    at_end
  };

public:
  WinDirIterImpl(path p)
    : _path(std::move(p))
    , _state(State::init)
  {
  }

  ~WinDirIterImpl() { close_handle(); }

  void close_handle()
  {
    if (_handle) {
      ::FindClose(_handle);
      _handle = 0;
    }
  }

  void increment(std::error_code& ec) override
  {
    WIN32_FIND_DATAW data;

    while (_state != State::at_end) {
      if (_state == State::init) {
        auto pattern = _path;
        pattern /= L"*.*";

        _handle = ::FindFirstFileW(pattern.c_str(), &data);
        if (_handle == INVALID_HANDLE_VALUE) {
          const auto err = ::GetLastError();
          if (err == ERROR_FILE_NOT_FOUND) {
            _state = State::at_end;
            close_handle();
            ec.clear();
          }
          else {
            ec = std::error_code(err, std::system_category());
          }
          return;
        }
        else {
          _state = State::cont;
        }
      }
      else {
        if (!::FindNextFileW(_handle, &data)) {
          auto err = ::GetLastError();
          if (err == ERROR_NO_MORE_FILES) {
            _state = State::at_end;
            close_handle();
            ec.clear();
            return;
          }
        }
      }

      if (::wcscmp(data.cFileName, L"..") == 0 || ::wcscmp(data.cFileName, L".") == 0) {
        continue;
      }

      _current.assign(_path / data.cFileName);
      ec.clear();
      return;
    }

    ec.clear();
  }

  const directory_entry& object() const override { return _current; }

  bool equal(const IDirIterImpl* other) const override
  {
    if (auto win_impl = dynamic_cast<const WinDirIterImpl*>(other)) {
      if (is_end() == win_impl->is_end()) {
        if (!is_end()) {
          return _current == win_impl->_current;
        }

        return true;
      }
    }

    return false;
  }

  bool is_end() const override { return _state == State::at_end; }

private:
  State _state = State::at_end;
  HANDLE _handle = 0;
  path _path;
  directory_entry _current;
};

}  // anon namespace


std::unique_ptr<directory_iterator::IDirIterImpl>
impl::make_dir_iterator(const path& p, std::error_code& ec)
{
  auto impl = estd::make_unique<WinDirIterImpl>(p);
  impl->increment(ec);
  if (ec || impl->is_end()) {
    return {};
  }
  return std::move(impl);
}


bool
impl::is_access_error(const std::error_code& ec)
{
  return ec.category() == std::system_category() && ec.value() == ERROR_ACCESS_DENIED;
}

}  // namespace filesystem
}  // namespace eyestep
