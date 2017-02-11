// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/operations.hpp"

#include "fspp/estd/optional.hpp"

#include <memory>
#include <vector>


namespace eyestep {
namespace filesystem {

inline directory_entry::directory_entry(const filesystem::path& p)
  : _path(p)
{
}


#if defined(FSPP_IS_VS2013)
inline directory_entry::directory_entry(directory_entry&& rhs)
  : _path(std::move(rhs._path))
  , _file_size(std::move(rhs._file_size))
{
}


inline directory_entry&
directory_entry::operator=(directory_entry&& rhs)
{
  _path = std::move(rhs._path);
  _file_size = std::move(rhs._file_size);
  return *this;
}
#endif


inline const path&
directory_entry::path() const
{
  return _path;
}


inline directory_entry::operator const filesystem::path&() const
{
  return _path;
}


inline file_status
directory_entry::status() const
{
  return filesystem::status(_path);
}


inline file_status
directory_entry::status(std::error_code& ec) const
{
  return filesystem::status(_path, ec);
}


inline file_status
directory_entry::symlink_status() const
{
  return filesystem::symlink_status(_path);
}


inline file_status
directory_entry::symlink_status(std::error_code& ec) const
{
  return filesystem::symlink_status(_path, ec);
}


inline file_size_type
directory_entry::file_size() const
{
  return _file_size ? _file_size.value() : filesystem::file_size(path());
}


inline file_size_type
directory_entry::file_size(std::error_code& ec) const NOEXCEPT
{
  return _file_size ? _file_size.value() : filesystem::file_size(path(), ec);
}


inline void
directory_entry::assign(const filesystem::path& p)
{
  _path = p;
  _file_size.reset();
}


inline void
directory_entry::assign(const filesystem::path& p, file_size_type file_size)
{
  _path = p;
  _file_size = file_size;
}


inline void
directory_entry::replace_filename(const filesystem::path& p)
{
  _path = _path.parent_path() / p;
  _file_size.reset();
}


inline bool
operator==(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() == rhs.path();
}


inline bool
operator!=(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() != rhs.path();
}


inline bool
operator<(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() < rhs.path();
}


inline bool
operator<=(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() <= rhs.path();
}


inline bool
operator>(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() > rhs.path();
}


inline bool
operator>=(const directory_entry& lhs, const directory_entry& rhs)
{
  return lhs.path() >= rhs.path();
}


//----------------------------------------------------------------------------------------

inline directory_iterator
begin(directory_iterator iter)
{
  return iter;
}


inline directory_iterator
end(const directory_iterator&)
{
  return directory_iterator();
}


//----------------------------------------------------------------------------------------

class FSPP_API recursive_directory_iterator::IImpl
{
public:
  struct Level
  {
    std::vector<directory_entry> entries;
    size_t idx = 0;
  };

  IImpl(directory_iterator first, directory_options options, std::error_code& ec);

  const directory_entry& current() const;
  void increment(std::error_code& ec);
  bool is_end() const;
  bool equal(const recursive_directory_iterator::IImpl* rhs) const;
  void pop();

  directory_options options() const { return _options; }
  bool recursion_pending() const { return _recursion_pending; }
  void set_recursion_pending(bool val) { _recursion_pending = val; }
  int depth() const
  {
    return static_cast<int>(
      std::distance(std::begin(const_cast<IImpl*>(this)->_stack), _stack_top) - 1);
  }

private:
  bool forward_to_first_file(std::error_code& ec);
  void pop_level();

  std::vector<Level> _stack;
  std::vector<Level>::iterator _stack_top;
  directory_iterator _iter;
  bool _recursion_pending = true;
  directory_options _options;
};


inline const directory_entry& recursive_directory_iterator::operator*() const
{
  return _impl->current();
}


inline const directory_entry* recursive_directory_iterator::operator->() const
{
  return &_impl->current();
}


inline directory_options
recursive_directory_iterator::options() const NOEXCEPT
{
  return _impl->options();
}


inline bool
recursive_directory_iterator::recursion_pending() const NOEXCEPT
{
  return _impl->recursion_pending();
}


inline void
recursive_directory_iterator::disable_recursion_pending()
{
  _impl->set_recursion_pending(false);
}


//----------------------------------------------------------------------------------------

inline recursive_directory_iterator
begin(recursive_directory_iterator iter)
{
  return iter;
}


inline recursive_directory_iterator
end(const recursive_directory_iterator&)
{
  return recursive_directory_iterator();
}

}  // namespace filesystem
}  // namespace eyestep
