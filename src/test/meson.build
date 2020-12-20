# -*-meson-*-

# Copyright (c) 2020 Gregor Klinke


fspptests_sources = [
  'main.cpp',
  'tst_canonical.cpp',
  'tst_dir_entry.cpp',
  'tst_dir_iter.cpp',
  'tst_operations.cpp',
  'tst_path.cpp',
  'tst_performance.cpp',
  'tst_types.cpp',
]

fspptests = executable('fspptests',
                       fspptests_sources,
                       include_directories : [catch_inc, fspp_inc],
                       cpp_args : fspp_args,
                       link_with: fspp_lib,
                       install : false)

test('tests', fspptests)
