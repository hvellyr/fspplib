// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/dir_iterator.hpp"
#include "fspp/details/filesystem_error.hpp"

#include "fspp/details/vfs.hpp"

#include "fspp/estd/memory.hpp"

#include "dir_iterator_private.hpp"
#include "vfs_private.hpp"

#include <system_error>
#include <utility>

namespace eyestep {
namespace filesystem {

namespace {
std::unique_ptr<directory_iterator::IDirIterImpl>
make_dir_iter_impl(const path& p, std::error_code& ec)
{
  if (auto val = vfs::with_vfs_do<std::unique_ptr<directory_iterator::IDirIterImpl>>(
        p, [&](vfs::IFilesystem& fs, const path&) {
          // we need the non-derooted path for the iterator.
          return fs.make_dir_iterator(p, ec);
        })) {
    return std::move(val.value());
  }

  return impl::make_dir_iterator(p, ec);
}


std::unique_ptr<directory_iterator::IDirIterImpl>
make_dir_iter_impl(const path& p)
{
  std::error_code ec;
  auto rv = make_dir_iter_impl(p, ec);
  if (ec) {
    throw filesystem_error("can't create directory iterator", p, ec);
  }

  return rv;
}

}  // anon namespace


directory_iterator::~directory_iterator() = default;


directory_iterator::directory_iterator() NOEXCEPT
{
}


directory_iterator::directory_iterator(const path& p)
  : _impl(std::shared_ptr<IDirIterImpl>(make_dir_iter_impl(p)))
{
}


directory_iterator::directory_iterator(const path& p, std::error_code& ec) NOEXCEPT
  : _impl(std::shared_ptr<IDirIterImpl>(make_dir_iter_impl(p, ec)))
{
}


#if defined(FSPP_IS_VS2013)
directory_iterator::directory_iterator(directory_iterator&& rhs)
  : _impl(std::move(rhs._impl))
{
}


directory_iterator&
directory_iterator::operator=(directory_iterator&& rhs)
{
  _impl = std::move(rhs._impl);
  return *this;
}
#endif


auto directory_iterator::operator*() const -> reference
{
  return _impl->object();
}


auto directory_iterator::operator-> () const -> pointer
{
  return &_impl->object();
}


directory_iterator& directory_iterator::operator++()
{
  std::error_code ec;
  _impl->increment(ec);
  if (ec) {
    throw filesystem_error("can't increment iterator", ec);
  }

  return *this;
}


directory_iterator&
directory_iterator::increment(std::error_code& ec) NOEXCEPT
{
  _impl->increment(ec);
  return *this;
}


bool
directory_iterator::operator==(const directory_iterator& rhs) const
{
  if (_impl == rhs._impl) {
    return true;
  }
  else if (_impl && !rhs._impl) {
    return _impl->is_end();
  }
  else if (!_impl && rhs._impl) {
    return rhs._impl->is_end();
  }
  else if (_impl && rhs._impl) {
    return _impl->equal(rhs._impl.get());
  }

  return false;
}


bool
directory_iterator::operator!=(const directory_iterator& rhs) const
{
  return !(*this == rhs);
}


//----------------------------------------------------------------------------------------

recursive_directory_iterator::IImpl::IImpl(directory_iterator first,
                                           directory_options options,
                                           std::error_code& ec)
  : _iter(std::move(first))
  , _options(options)
{
  using std::end;

  _stack.emplace_back();  // sentinel
  _stack.emplace_back();
  _stack_top = std::prev(end(_stack));

  forward_to_first_file(ec);
}


bool
recursive_directory_iterator::IImpl::forward_to_first_file(std::error_code& ec)
{
  while (_iter != end(_iter)) {
    const auto st = _iter->status(ec);
    if (!is_directory(st)) {
      break;
    }

    const auto sym_st = _iter->symlink_status(ec);
    if (ec) {
      return false;
    }
    if (!is_symlink(sym_st)
        || (_options & directory_options::follow_directory_symlink) != 0) {
      _stack_top->entries.emplace_back(*_iter);
    }
    else {
      return true;
    }

    _iter.increment(ec);
    if (ec) {
      return false;
    }
  }

  if (ec) {
    return false;
  }

  return true;
}


void
recursive_directory_iterator::IImpl::pop_level()
{
  using std::begin;

  while (_stack_top != begin(_stack) && _stack_top->idx >= _stack_top->entries.size()) {
    --_stack_top;

    if (_stack_top != begin(_stack)) {
      _stack_top->idx++;
    }
  }
  // current item is now a folder or the END
}


void
recursive_directory_iterator::IImpl::increment(std::error_code& ec)
{
  const auto was_recursion_pending = _recursion_pending;
  _recursion_pending = true;

  const auto step_forward = [&](std::error_code& ec2) {
    if (!forward_to_first_file(ec2)) {
      return;
    }

    if (_iter != end(_iter)) {
      // current item is now _iter.current();
      return;
    }

    if (_stack_top->idx < _stack_top->entries.size()) {
      // current item is now a folder or the END
      return;
    }

    pop_level();
  };


  if (_iter != end(_iter)) {
    _iter.increment(ec);
    if (ec) {
      return;
    }
    return step_forward(ec);
  }

  if (_stack_top->idx < _stack_top->entries.size()) {
    if (was_recursion_pending) {
      auto it = directory_iterator(_stack_top->entries[_stack_top->idx].path(), ec);
      if (!ec) {
        _iter = std::move(it);

        ++_stack_top;
        if (_stack_top != end(_stack)) {
          *_stack_top = Level{};
        }
        else {
          _stack.emplace_back();
          _stack_top = std::prev(end(_stack));
        }
      }
      else if (impl::is_access_error(ec)
               && (_options & directory_options::skip_permission_denied) != 0) {
        ec.clear();
        _stack_top->idx++;
      }
      else {
        return;
      }
    }
    else {
      _stack_top->idx++;
    }

    return step_forward(ec);
  }

  pop_level();
}


bool
recursive_directory_iterator::IImpl::is_end() const
{
  using std::begin;

  return _iter == end(_iter) && _stack_top == begin(_stack);
}


const directory_entry&
recursive_directory_iterator::IImpl::current() const
{
  return _iter != end(_iter) ? *_iter : _stack_top->entries[_stack_top->idx];
}


bool
recursive_directory_iterator::IImpl::equal(
  const recursive_directory_iterator::IImpl* rhs) const
{
  if (auto impl = dynamic_cast<const recursive_directory_iterator::IImpl*>(rhs)) {
    if (is_end() == impl->is_end()) {
      if (!is_end()) {
        return current() == impl->current();
      }

      return true;
    }
  }

  return false;
}


void
recursive_directory_iterator::IImpl::pop()
{
  using std::begin;

  _iter = directory_iterator();

  if (_stack_top != begin(_stack)) {
    --_stack_top;

    if (_stack_top != begin(_stack)) {
      _stack_top->idx++;
    }

    pop_level();
  }
  // current item is now a folder or the END
}


namespace {

std::unique_ptr<recursive_directory_iterator::IImpl>
make_recursive_dir_iter_impl(const path& p,
                             directory_options options,
                             std::error_code& ec)
{
  using namespace std;

  auto iter = directory_iterator(p, ec);
  if (ec) {
    return {};
  }

  if (iter != end(iter)) {
    auto impl = estd::make_unique<recursive_directory_iterator::IImpl>(
      std::move(iter), options, ec);
    if (ec) {
      return {};
    }
    return impl;
  }

  return {};
}


std::unique_ptr<recursive_directory_iterator::IImpl>
make_recursive_dir_iter_impl(const path& p, directory_options options)
{
  std::error_code ec;
  auto rv = make_recursive_dir_iter_impl(p, options, ec);
  if (ec) {
    throw filesystem_error("can't create directory iterator", p, ec);
  }

  return rv;
}
}  // anon namespace


recursive_directory_iterator::recursive_directory_iterator() NOEXCEPT
{
}


recursive_directory_iterator::recursive_directory_iterator(const path& p,
                                                           directory_options options)
  : _impl(std::shared_ptr<IImpl>(make_recursive_dir_iter_impl(p, options)))
{
}


recursive_directory_iterator::recursive_directory_iterator(const path& p,
                                                           directory_options options,
                                                           std::error_code& ec) NOEXCEPT
  : _impl(std::shared_ptr<IImpl>(make_recursive_dir_iter_impl(p, options, ec)))
{
}


recursive_directory_iterator::recursive_directory_iterator(const path& p,
                                                           std::error_code& ec) NOEXCEPT
  : _impl(std::shared_ptr<IImpl>(
      make_recursive_dir_iter_impl(p, directory_options::none, ec)))
{
}


#if defined(FSPP_IS_VS2013)
recursive_directory_iterator::recursive_directory_iterator(
  recursive_directory_iterator&& rhs)
  : _impl(std::move(rhs._impl))
{
}


recursive_directory_iterator&
recursive_directory_iterator::operator=(recursive_directory_iterator&& rhs)
{
  _impl = std::move(rhs._impl);
  return *this;
}
#endif


recursive_directory_iterator& recursive_directory_iterator::operator++()
{
  std::error_code ec;
  _impl->increment(ec);
  if (ec) {
    throw filesystem_error("can't increment iterator", ec);
  }

  return *this;
}


recursive_directory_iterator&
recursive_directory_iterator::increment(std::error_code& ec) NOEXCEPT
{
  _impl->increment(ec);
  return *this;
}


void
recursive_directory_iterator::pop()
{
  _impl->pop();
}


int
recursive_directory_iterator::depth() const
{
  return _impl->depth();
}


bool
recursive_directory_iterator::operator==(const recursive_directory_iterator& rhs) const
{
  if (_impl == rhs._impl) {
    return true;
  }
  else if (_impl && !rhs._impl) {
    return _impl->is_end();
  }
  else if (!_impl && rhs._impl) {
    return rhs._impl->is_end();
  }
  else if (_impl && rhs._impl) {
    return _impl->equal(rhs._impl.get());
  }

  return false;
}


bool
recursive_directory_iterator::operator!=(const recursive_directory_iterator& rhs) const
{
  return !(*this == rhs);
}

}  // namespace filesystem
}  // namespace eyestep
