// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"
#include "fspp/details/platform.hpp"

#if defined(FSPP_HAVE_STD_CODECVT)
#include <codecvt>
#endif
#include <iterator>
#include <locale>
#include <string>


namespace eyestep {
namespace filesystem {
namespace detail {

#if defined(FSPP_SUPPORT_WSTRING_API)
inline std::wstring&
convert(std::wstring& dst, const std::basic_string<wchar_t>& src)
{
  dst = src;
  return dst;
}


inline std::wstring&
convert(std::wstring& dst, const std::string& src)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv16;
  dst = conv16.from_bytes(src);
  return dst;
}


inline std::wstring&
convert(std::wstring& dst, const char* src)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv16;
  dst = conv16.from_bytes(src);
  return dst;
}


inline std::string&
convert(std::string& dst, const std::wstring& src)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv8;
  dst = conv8.to_bytes(src);
  return dst;
}
#endif


inline std::string&
convert(std::string& dst, const std::string& src)
{
  dst = src;
  return dst;
}


inline std::string&
convert(std::string& dst, const char* src)
{
  dst = src;
  return dst;
}


template <typename T, typename InputIt>
inline T&
convert(T& dst, InputIt i_first, InputIt i_last)
{
  if (i_first != i_last) {
    auto str = std::basic_string<typename std::iterator_traits<InputIt>::value_type>{
      i_first, i_last};
    T tmp;
    detail::convert(tmp, str);
    dst.swap(tmp);
  }

  return dst;
}

}  // namespace detail
}  // namespace filesystem
}  // namespace eyestep
