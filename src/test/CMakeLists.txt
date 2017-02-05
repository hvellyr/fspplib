# Copyright (c) 2016 Gregor Klinke

add_executable(fspplib_tests
  main.cpp
)

target_include_directories(fspplib_tests PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/third-party")

target_link_libraries(fspplib_tests
  fspplib)


add_test(FsppLib_TestCase fspplib_tests)
