# Copyright (c) 2016 Gregor Klinke

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG=1)
else()
  add_definitions(-DNDEBUG=1)
endif()

set(posix_sources
    posix/dir_iterator_posix.cpp
    posix/operations_posix.cpp
    posix/limits_posix.cpp
    )

if(WIN32)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  set(platform_sources
    win/dir_iterator_win.cpp
    win/operations_win.cpp
    win/limits_win.cpp
    )
elseif(APPLE)
  set(platform_sources
    mac/operations_mac.cpp
    ${posix_sources}
    )
elseif(UNIX)
  set(platform_sources
    unix/operations_unix.cpp
    ${posix_sources}
    )
endif()

add_library(fspplib
  common.cpp
  common.hpp
  dir_iterator.cpp
  dir_iterator_private.hpp
  file.cpp
  include/fspp/details/dir_iterator.hpp
  include/fspp/details/dir_iterator.ipp
  include/fspp/details/file.hpp
  include/fspp/details/file.ipp
  include/fspp/details/file_status.hpp
  include/fspp/details/filesystem_error.hpp
  include/fspp/details/operations.hpp
  include/fspp/details/path.hpp
  include/fspp/details/path.ipp
  include/fspp/details/path_convert.hpp
  include/fspp/details/platform.hpp
  include/fspp/details/types.hpp
  include/fspp/details/vfs.hpp
  include/fspp/details/vfs.ipp
  include/fspp/estd/algorithm.hpp
  include/fspp/estd/memory.hpp
  include/fspp/estd/optional.hpp
  include/fspp/estd/type_traits.hpp
  include/fspp/filesystem.hpp
  include/fspp/limits.hpp
  include/fspp/utility/bitmask_type.hpp
  include/fspp/utility/scope.hpp
  include/fspp/utils.hpp
  include/fspp/utils.ipp
  include/fspp/limits.hpp
  memory_vfs.cpp
  memory_vfs.hpp
  operations.cpp
  operations_impl.hpp
  path.cpp
  utils.cpp
  vfs.cpp
  vfs_private.hpp
  ${platform_sources}
)
target_include_directories(fspplib PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src/include;${CMAKE_CURRENT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/third-party>")
target_compile_options(fspplib
  PUBLIC ${cxx11_options} ${warning_options})

install(TARGETS fspplib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(DIRECTORY "include/" "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src/include/>"
        DESTINATION include
        FILES_MATCHING PATTERN "*.[hi]pp")

add_subdirectory(test)
