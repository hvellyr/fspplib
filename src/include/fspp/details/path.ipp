// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/platform.hpp"

#include <algorithm>
#include <cctype>
#include <istream>
#include <ostream>
#include <string>


namespace eyestep {
namespace filesystem {

inline path::path(const std::string& source)
{
  detail::convert(_data, source);
}


inline path::path(const char* source)
{
  detail::convert(_data, source);
}


#if defined(FSPP_SUPPORT_WSTRING_API)
inline path::path(const std::wstring& source)
{
  detail::convert(_data, source);
}


inline path::path(const wchar_t* source)
{
  detail::convert(_data, source);
}
#endif


template <typename Source>
inline path::path(const Source& source)
{
  detail::convert(_data, source);
}


template <class InputIt>
inline path::path(InputIt i_first, InputIt i_last)
{
  detail::convert(_data, i_first, i_last);
}


inline auto
path::begin() const -> iterator
{
  return iterator(*this, std::begin(_data));
}


inline auto
path::end() const -> iterator
{
  return iterator(*this, std::end(_data));
}


#if defined(FSPP_IS_VS2013)
inline path::path(path&& other)
  : _data(std::move(other._data))
{
}


inline path&
path::operator=(path&& other)
{
  _data = std::move(other._data);
  return *this;
}
#endif


template <typename Source>
inline path&
path::operator=(const Source& source)
{
  return this->assign(source);
}


template <typename Source>
inline path&
path::assign(const Source& source)
{
  string_type data;
  detail::convert(data, source);
  _data.swap(data);
  return *this;
}


template <typename InputIt>
inline path&
path::assign(InputIt i_first, InputIt i_last)
{
  detail::convert(_data, i_first, i_last);
  return *this;
}


template <class Source>
inline path&
path::operator/=(const Source& source)
{
  return operator/=(path(source));
}


template <class Source>
inline path&
path::append(const Source& source)
{
  return operator/=(source);
}


template <typename InputIt>
inline path&
path::append(InputIt i_first, InputIt i_last)
{
  return operator/=(path(i_first, i_last));
}


inline path&
path::operator+=(const path& p)
{
  _data += p.native();
  return *this;
}


inline path&
path::operator+=(const string_type& str)
{
  _data += str;
  return *this;
}


inline path&
path::operator+=(const value_type* ptr)
{
  _data += ptr;
  return *this;
}


template <typename CharT, typename std::enable_if<std::is_integral<CharT>::value>::type*>
inline path&
path::operator+=(CharT c)
{
  CharT t[2] = {c, 0};
  string_type p;
  detail::convert(p, t);
  _data += p;
  return *this;
}


template <class Source, typename std::enable_if<!std::is_integral<Source>::value>::type*>
inline path&
path::operator+=(const Source& source)
{
  string_type p;
  detail::convert(p, source);
  _data += p;
  return *this;
}


template <class Source>
inline path&
path::concat(const Source& source)
{
  return operator+=(path(source));
}


inline void
path::clear()
{
  _data.clear();
}


inline bool
path::empty() const
{
  return _data.empty();
}


inline path&
path::make_preferred()
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  using std::begin;
  using std::end;

  const auto ps = preferred_separator;
  std::replace(begin(_data), end(_data), static_cast<value_type>('/'), ps);
#endif
  return *this;
}


inline void
path::swap(path& other)
{
  _data.swap(other._data);
}


inline auto
path::c_str() const -> const value_type*
{
  return native().c_str();
}


inline auto
path::native() const -> const string_type&
{
  return _data;
}


inline path::operator string_type() const
{
  return _data;
}


inline std::string
path::string() const
{
  return u8string();
}


inline std::string
path::u8string() const
{
  std::string result;
  return detail::convert(result, _data);
}


inline std::string
path::generic_string() const
{
  return generic_u8string();
}


inline std::string
path::generic_u8string() const
{
  using std::begin;
  using std::end;

  auto tmp = _data;
  std::replace(begin(tmp), end(tmp), '\\', '/');

  std::string result;
  return detail::convert(result, tmp);
}


#if defined(FSPP_SUPPORT_WSTRING_API)
inline std::wstring
path::wstring() const
{
  std::wstring result;
  return detail::convert(result, _data);
}


inline std::wstring
path::generic_wstring() const
{
  using std::begin;
  using std::end;

  auto tmp = _data;
  std::replace(begin(tmp), end(tmp), '\\', '/');

  std::wstring result;
  return detail::convert(result, tmp);
}
#endif


inline int
path::compare(const string_type& str) const
{
  return compare(path(str));
}


inline int
path::compare(const value_type* s) const
{
  return compare(path(s));
}


inline path&
path::remove_filename()
{
  *this = parent_path();
  return *this;
}


inline path&
path::replace_filename(const path& replacement)
{
  *this = parent_path() / replacement;
  return *this;
}


inline bool
path::is_absolute() const
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  return has_root_name() && has_root_directory();
#else
  return has_root_directory();
#endif
}


inline bool
path::is_relative() const
{
  return !is_absolute();
}


inline bool
operator==(const path& lhs, const path& rhs)
{
  return lhs.compare(rhs) == 0;
}


inline bool
operator!=(const path& lhs, const path& rhs)
{
  return !operator==(lhs, rhs);
}


inline bool
operator<(const path& lhs, const path& rhs)
{
  return lhs.compare(rhs) < 0;
}


inline bool
operator<=(const path& lhs, const path& rhs)
{
  return lhs.compare(rhs) <= 0;
}


inline bool
operator>(const path& lhs, const path& rhs)
{
  return lhs.compare(rhs) > 0;
}


inline bool
operator>=(const path& lhs, const path& rhs)
{
  return lhs.compare(rhs) >= 0;
}


inline path
operator/(const path& lhs, const path& rhs)
{
  return path(lhs) /= rhs;
}


inline void
swap(path& lhs, path& rhs)
{
  lhs.swap(rhs);
}


template <class Source>
inline path
u8path(const Source& source)
{
  return path(source);
}


template <class InputIt>
path
u8path(InputIt i_first, InputIt i_last)
{
  return path(i_first, i_last);
}


//----------------------------------------------------------------------------------------

#if defined(FSPP_IS_VS2013)
inline path::iterator::iterator(iterator&& rhs)
  : _data(std::move(rhs._data))
  , _it(std::move(rhs._it))
  , _elt(std::move(rhs._elt))
{
}


inline auto
path::iterator::operator=(iterator&& rhs) -> iterator&
{
  _data = std::move(rhs._data);
  _it = std::move(rhs._it);
  _elt = std::move(rhs._elt);
  return *this;
}
#endif


inline bool
path::iterator::operator==(const iterator& rhs) const
{
  return _data == rhs._data && _it == rhs._it;
}


inline bool
path::iterator::operator!=(const iterator& rhs) const
{
  return !(operator==(rhs));
}


inline auto path::iterator::operator--(int) -> iterator
{
  auto i_tmp = iterator(*this);
  operator--();
  return i_tmp;
}


inline auto path::iterator::operator++(int) -> iterator
{
  auto i_tmp = iterator(*this);
  operator++();
  return i_tmp;
}


inline auto path::iterator::operator*() -> reference
{
  return _elt;
}


inline auto path::iterator::operator-> () -> pointer
{
  return &_elt;
}


//----------------------------------------------------------------------------------------

inline ::std::ostream&
operator<<(::std::ostream& os, const path& p)
{
  os << p.u8string().c_str();
  return os;
}


inline ::std::istream&
operator>>(::std::istream& is, path& p)
{
  std::string tmp;
  is >> tmp;
  p = path(tmp);
  return is;
}


#if defined(FSPP_SUPPORT_WSTRING_API)
inline ::std::wostream&
operator<<(::std::wostream& os, const path& p)
{
  os << p.wstring().c_str();
  return os;
}


inline ::std::wistream&
operator>>(::std::wistream& is, path& p)
{
  std::wstring tmp;
  is >> tmp;
  p = path(tmp);
  return is;
}
#endif

}  // namespace filesystem
}  // namespace eyestep
