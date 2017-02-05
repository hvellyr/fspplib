# Copyright (c) 2016 Gregor Klinke

cmake_minimum_required(VERSION 2.8.12)

project(FSPP CXX)
set(FSPP_VERSION_MAJOR 0)
set(FSPP_VERSION_MINOR 1)

# Enable C++11
include(CheckCXXCompilerFlag)
if(WIN32)
  # Check if we are using Visual Studio 2013 or later
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
    message(FATAL_ERROR "You are using an unsupported Windows compiler! (Visual C++ 2013 or later required)")
  endif()
  set(cxx11_options "/EHsc")
else()
  set(cxx11_options -std=c++11)
  CHECK_CXX_COMPILER_FLAG(${cxx11_options} COMPILER_SUPPORTS_CXX11)

  if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has insufficient C++11 support. Please use a different C++ compiler.")
  endif()

  set(platform_options -EYE_ISWIN)

endif()

# Build debug by default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang
    OR CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
  set(warning_options
    -Werror
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-disabled-macro-expansion
    -Wno-documentation-unknown-command
    -Wno-exit-time-destructors
    -Wno-padded
    -Wno-undefined-reinterpret-cast
    -Wno-documentation-deprecated-sync
    -Wno-weak-vtables
    -Wno-global-constructors
    )
elseif(MSVC)
  set(warning_options /WX /W4 /wd4275 /wd4503 /wd4512 /wd4714)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_MACOSX_RPATH ON)

# Check for some system header files
# This only works for C headers, sigh.
# INCLUDE (CheckIncludeFiles)
# CHECK_INCLUDE_FILES (codecvt HAVE_STD_CODECVT)
INCLUDE (CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "${cxx11_options}")
CHECK_CXX_SOURCE_COMPILES("
  #include <codecvt>
  int main() {
    std::codecvt_utf8_utf16<char16_t> x;
    return 0;
  }
" FSPP_HAVE_STD_CODECVT)

CHECK_CXX_SOURCE_COMPILES("
  #include <sstream>
  int main() {
    auto _stream = std::stringstream(std::ios::in);
    return 0;
  }
" FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN)

CHECK_CXX_SOURCE_COMPILES("
  #include <algorithm>
  #include <vector>
  int main() {
    auto a = std::vector<int>{};
    auto b = std::vector<int>{};
    auto f = std::mismatch(
      std::begin(a), std::end(a), std::begin(b), std::end(b),
      [](int i, int j) { return i == j; });
    return 0;
  }
" FSPP_HAVE_STD_MISMATCH_WITH_PREDICATE)

CHECK_CXX_SOURCE_COMPILES("
  #include <memory>
  #include <string>
  int main() {
    auto f = std::make_unique<std::string>();
    return 0;
  }
" FSPP_HAVE_STD_MAKE_UNIQUE)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/include/fspp/details/config.hpp.in
               ${PROJECT_BINARY_DIR}/src/include/fspp/details/config.hpp)

enable_testing()

include(FeatureSummary)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(src)
