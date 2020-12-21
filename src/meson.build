# -*-meson-*-

# Copyright (c) 2020 Gregor Klinke


configure_file(input : 'include/fspp/details/config.hpp.in-meson',
               output : 'fspp-config.hpp',
               configuration : conf_data,
               install : not meson.is_subproject(),
               install_dir : 'include/fspp/details')

fspp_inc = include_directories('.', 'include')

fspp_args = ['-DUSE_FSPP_CONFIG_HPP=1']

fspp_sources = [
  'common.cpp',
  'dir_iterator.cpp',
  'file.cpp',
  'memory_vfs.cpp',
  'operations.cpp',
  'path.cpp',
  'utils.cpp',
  'vfs.cpp',
]

if host_machine.system() == 'darwin'
  fspp_sources += [
    'posix/dir_iterator_posix.cpp',
    'posix/operations_posix.cpp',
    'posix/limits_posix.cpp',
    'mac/operations_mac.cpp',
  ]
elif host_machine.system() == 'linux'
  fspp_sources += [
    'posix/dir_iterator_posix.cpp',
    'posix/operations_posix.cpp',
    'posix/limits_posix.cpp',
    'unix/operations_unix.cpp',
  ]
elif host_machine.system() == 'windows'
  fspp_sources += [
    'dir_iterator_win.cpp',
    'operations_win.cpp',
    'limits_win.cpp',
  ]
endif


fspp_lib = static_library('fspplib',
                          fspp_sources,
                          include_directories : [fspp_inc],
                          cpp_args : fspp_args,
                          install : not meson.is_subproject())

fspp_dep = declare_dependency(
  include_directories : fspp_inc,
  compile_args : fspp_args,
  link_with : fspp_lib)


if not meson.is_subproject()
  install_subdir('include',
                 install_dir : '',
                 strip_directory : false,
                 exclude_files : ['fspp/details/config.hpp.in',
                                  'fspp/details/config.hpp.in-meson'])
endif

subdir('test')
