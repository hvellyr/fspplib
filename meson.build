# -*-meson-*-

# Copyright (c) 2020 Gregor Klinke


project('fspplib', ['cpp'],
        version : '0.1',
        license : 'BSD',
        meson_version : '>= 0.54',
        default_options : ['cpp_std=c++11',
                           'buildtype=release'])

set_variable('copyright_year', '2017-2020')
set_variable('copyright_owner', 'Gregor C. Klinke')

if get_option('buildtype') == 'debug'
  add_project_arguments('-DIS_DEBUG=1', language: 'cpp')
endif

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('copyright_year', copyright_year)
conf_data.set('copyright_owner', copyright_owner)

prefix = get_option('prefix')
conf_data.set('prefix', prefix)
conf_data.set('bindir', prefix / get_option('bindir'))
conf_data.set('libdir', prefix / get_option('libdir'))
conf_data.set('pkglibdir', prefix / get_option('libdir') / 'fspp')
conf_data.set('includedir', prefix / get_option('includedir'))
conf_data.set('pkgincludedir', prefix / get_option('includedir') / 'fspp')
conf_data.set('datadir', prefix / get_option('datadir'))

pkgdatadir = prefix / get_option('datadir') / 'fspp'
conf_data.set('pkgdatadir', pkgdatadir)

if host_machine.system() == 'darwin'
  conf_data.set('OS_mac', 1)
elif host_machine.system() == 'windows'
  conf_data.set('OS_win', 1)
elif host_machine.system() == 'linux'
  conf_data.set('OS_linux', 1)
endif


# ------------------------------------------------------------------------------

cc = meson.get_compiler('cpp')

if cc.compiles('''#include <optional>

int main() {
  auto o = std::optional<int>{42};
  return *o;
}''',
               name : 'std::optional available')
  conf_data.set('FSPP_HAVE_STD_OPTIONAL', 1)
endif


if cc.compiles('''#include <memory>

class Foo { public: int m = 42; };

int main() {
  auto o = std::make_unique<Foo>();
  return o->m == 42 ? 0 : 1;
}
''',
               name : 'std::make_unique available')
  conf_data.set('FSPP_HAVE_STD_MAKE_UNIQUE', 1)
endif


if cc.compiles('''#include <codecvt>
#include <locale>

int main() {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv16;
  std::wstring dst = conv16.from_bytes(std::string("hello world"));

  return dst.size() != 0 ? 0 : 1;
}
''',
               name : 'std::codecvt available')
  conf_data.set('FSPP_HAVE_STD_CODECVT', 1)
endif


if cc.compiles('''#include <type_traits>

template <typename T, typename U>
auto
foo(T t) -> std::enable_if_t<!std::is_array<T>::value, U>
{
  return static_cast<U>(t);
}

int main() {
  return 0;
}
''',
               name : 'std::enable_if_t available')
  conf_data.set('FSPP_HAVE_STD_ENABLE_IF_T', 1)
endif


if cc.compiles('''#include <sstream>

int main() {
  std::stringstream ss;
  ss << "foo";
  ss = std::stringstream("hello world", std::ios::out);
  return 0;
}
''',
               name : 'std::stringstream supports copy assignment')
  conf_data.set('FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN', 1)
endif


if cc.compiles('''#include <algorithm>
#include <vector>
int main() {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto b = std::vector<int>{1, 2};
  return mismatch(begin(v), end(v), begin(b), end(b),
                  [](int a, int b) {
                    return a == b;
                  }).first == begin(v);
}
''',
               name : 'std::mismatch has a predicate')
  conf_data.set('FSPP_HAVE_STD_MISMATCH_WITH_PREDICATE', 1)
endif


# ------------------------------------------------------------------------------

catch_inc = include_directories('third-party')

subdir('src')
