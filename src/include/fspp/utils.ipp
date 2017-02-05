// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/file.hpp"
#include "fspp/utility/scope.hpp"

#include <cassert>
#include <istream>
#include <ostream>
#include <string>
#include <system_error>


namespace eyestep {
namespace filesystem {

template <typename Functor, typename StreamType>
void
with_stream(File file, std::ios::openmode mode, std::error_code& ec, Functor functor)
{
  auto file_guard = utility::make_scope([&]() { file.close(ec); });
  auto& is = file.open(mode, ec);
  if (!ec) {
    functor(static_cast<StreamType&>(is));
  }
}


template <typename Functor, typename StreamType>
void
with_stream(File file, std::ios::openmode mode, Functor functor)
{
  std::error_code ec;
  with_stream(file, mode, ec, functor);
  if (ec) {
    throw filesystem_error("Failed opening file", file.path(), ec);
  }
}


template <typename Functor>
void
with_stream_for_reading(File file, std::error_code& ec, Functor functor)
{
  with_stream<Functor, std::istream>(file, std::ios::in, ec, functor);
}


template <typename Functor>
void
with_stream_for_reading(File file, Functor functor)
{
  std::error_code ec;
  with_stream<Functor, std::istream>(file, std::ios::in, ec, functor);
  if (ec) {
    throw filesystem_error("Failed reading file", file.path(), ec);
  }
}


template <typename Functor>
void
with_stream_for_writing(File file,
                        std::error_code& ec,
                        Functor functor,
                        std::ios::openmode addmode)
{
  with_stream<Functor, std::ostream>(file, std::ios::out | addmode, ec, functor);
}


template <typename Functor>
void
with_stream_for_writing(File file, Functor functor, std::ios::openmode addmode)
{
  std::error_code ec;
  with_stream<Functor, std::ostream>(file, std::ios::out | addmode, ec, functor);
  if (ec) {
    throw filesystem_error("Failed writing file", file.path(), ec);
  }
}


//----------------------------------------------------------------------------------------

template <typename Functor>
void
with_temp_dir(Functor functor)
{
  auto tmp_dir = create_temp_dir(temp_directory_path());
  auto guard = utility::make_scope([&]() {
    if (!tmp_dir.empty() && exists(tmp_dir)) {
      remove_all(tmp_dir);
    }
  });

  functor(tmp_dir);
}

}  // namespace filesystem
}  // namespace eyestep
