FS++Lib
=======

What is it?
-----------

FS++Lib is an implementation following the the Filesystem Library technical
specification from the C++17 standard.  See here
[http://en.cppreference.com/w/cpp/filesystem](http://en.cppreference.com/w/cpp/filesystem)
for details.

It compiles on a c++11 compiler.


Depedencies and supported compilers
-----------------------------------

Currently known to build with

- clang-3.7.1 on Mac
- GCC 4.9.1 on Ubuntu 14.04
- Visual Studio 2015 on Win10.


How to build
------------

The library can be build with [CMake](https://cmake.org/) or
[Meson](https://mesonbuild.com/).

With CMake:

```
$ mkdir output
$ cd output
$ cmake ..
$ cmake --build .
$ ctest -V
$ cmake --install . --prefix <prefix>
```
With Meson:

```
$ meson setup --prefix=<prefix>  output
$ cd output
$ meson compile
$ meson test
$ meson install
```

Known Issues
------------

Not ready yet.


License
-------

Distributed under a modified BSD license.  See [LICENSE](LICENSE) for details.


CI Status
---------
[![Build status](https://ci.appveyor.com/api/projects/status/m6qkc7co91bdolpp/branch/master?svg=true)](https://ci.appveyor.com/project/hvellyr/fspplib/branch/master)
[![Build Status](https://travis-ci.org/hvellyr/fspplib.svg?branch=master)](https://travis-ci.org/hvellyr/fspplib)
