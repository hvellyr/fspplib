# Copyright (c) 2016 Gregor Klinke

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG=1)
else()
  add_definitions(-DNDEBUG=1)
endif()

set(posix_sources
    )

if(WIN32)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  set(platform_sources
    )
elseif(APPLE)
  set(platform_sources
    ${posix_sources}
    )
elseif(UNIX)
  set(platform_sources
    ${posix_sources}
    )
endif()

add_library(fspplib
  include/fspp/details/platform.hpp
  ${platform_sources}
)
target_include_directories(fspplib PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src/include;${CMAKE_CURRENT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/third-party>")
target_compile_options(fspplib
  PUBLIC ${cxx11_options} ${warning_options})

add_subdirectory(test)
