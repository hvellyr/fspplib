// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#if defined(_WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define FSPP_IS_WIN 1
#elif defined(__APPLE__)
#define FSPP_IS_MAC 1
#else
#define FSPP_IS_UNIX 1
#endif


// TODO: define it properly in c++14
#if defined(__clang__)
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#else
#define CONSTEXPR
#define NOEXCEPT
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define FSPP_IS_VS2013 1
#endif

#define FSPP_API

// Older GCC don't have the <codecvt> header.  For simplicity disable wstring
// support for these platforms.
#if defined(FSPP_HAVE_STD_CODECVT)
#define FSPP_SUPPORT_WSTRING_API 1
#endif
