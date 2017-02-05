# Copyright (c) 2016 Gregor Klinke

# Check for some system header files
# This only works for C headers, sigh.
# include(CheckIncludeFiles)
# CHECK_INCLUDE_FILES (codecvt HAVE_STD_CODECVT)
include(CheckCXXSourceCompiles)

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


CHECK_CXX_SOURCE_COMPILES("
  #include <type_traits>
  template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  void foo() { }
  int main() {
    foo<int>();
    return 0;
  }
" FSPP_HAVE_STD_ENABLE_IF_T)


CHECK_CXX_SOURCE_COMPILES("
  #include <optional>
  int main() {
    auto f = std::optional<int>();
    return 0;
  }
" FSPP_HAVE_STD_OPTIONAL)
