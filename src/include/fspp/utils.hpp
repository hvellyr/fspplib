// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/file.hpp"
#include "fspp/details/path.hpp"
#include "fspp/filesystem.hpp"

#include <string>
#include <system_error>


namespace eyestep {
namespace filesystem {
class File;

/*! Opens @p file with mode @p mode and calls @p functor with a stream to the open file.
 *
 * When @p functor returns (either normally or by exception), the stream is closed.
 *
 * The signature of @p functor should be equivalent to `void fun(std::iostream&)`
 *
 * @code
 * std::string foo;
 * with_stream("foo.txt", std::ios::in | std::ios::out | std::ios::trunc,
 *             [&](std::iostream& os)
 *             {
 *               os << "foo";
 *               os.seekg(0);
 *               is >> foo;
 *             });
 * @endcode
 *
 * If you only need to read or write a file you may use with_stream_for_reading() or
 * with_stream_for_writing() to save typing the ios flags.  This operation is though
 * usefull for in-out stream.
 */
template <typename Functor, typename StreamType = std::iostream>
void
with_stream(File file, std::ios::openmode mode, std::error_code& ec, Functor functor);

/*! Opens @p file with mode @p mode and calls @p functor with a stream to the open file.
 *
 * @throws filesystem_error in case of an error
 */
template <typename Functor, typename StreamType = std::iostream>
void
with_stream(File file, std::ios::openmode mode, Functor functor);

/*! Opens @p file for reading and calls @p functor with a stream to the open file.
 *
 * When @p functor returns (either normally or by exception), the stream is closed.
 *
 * The signature of @p functor should be equivalent to `void fun(std::istream&)`
 *
 * @code
 * std::string foo;
 * with_stream_for_reading("foo.txt", [&](std::istream& os)
 *                         {
 *                           is >> foo;
 *                         });
 * @endcode
 */
template <typename Functor>
void
with_stream_for_reading(File file, std::error_code& ec, Functor functor);

/*! Opens @p file for reading and calls @p functor with a stream to the open file.
 *
 * @throws filesystem_error in case of an error
 */
template <typename Functor>
void
with_stream_for_reading(File file, Functor functor);

/*! Opens @p file for writing and calls @p functor with a stream to the open file.
 *
 * When @p functor returns (either normally or by exception), the stream is closed.
 *
 * @p addmode is set to std::ios::out by default, but by passing other flags the behaviour
 * can be changed (like std::ios::app for appending to the end of a file, std::ios::binary
 * for opening the stream in binary mode).
 *
 * The signature of @p functor should be equivalent to `void fun(std::ostream&)`
 *
 * @code
 * with_stream_for_writing("foo.txt", [](std::ostream& os)
 *                         {
 *                           os << "hello world!";
 *                         });
 * @endcode
 */
template <typename Functor>
void
with_stream_for_writing(File file,
                        std::error_code& ec,
                        Functor functor,
                        std::ios::openmode addmode = std::ios::out | std::ios::binary);

/*! Opens @p file for writing and calls @p functor with a stream to the open file.
 *
 * @throws filesystem_error in case of an error
 */
template <typename Functor>
void
with_stream_for_writing(File file,
                        Functor functor,
                        std::ios::openmode addmode = std::ios::out | std::ios::binary);

/*! Create a temporary folder with a unique name in @p temp_p.
 *
 * The folder is not removed automatically after the process exits.
 * See with_temp_dir() for a possible utility.
 *
 * @returns the path to the created folder. */
FSPP_API path
create_temp_dir(const path& temp_p, const std::string& prefix = "temp");

/*! Creates a temporary folder and calls @p functor with that folder
 *  as only parameter.  After returning from @p proc the temporary
 *  folder is removed.
 *
 * The signature of @p functor should be equivalent to `void fun(const
 * fs::path& tmpdir)`
 *
 * @code
 * with_temp_dir([](const fs::path& tmpdir) {
 *                   fs::touch(tmpdir / "foo");
 *                  });
 * @endcode
 */
template <typename Functor>
void
with_temp_dir(Functor functor);

/*! Writes @p data to a file at @p p.  If @p p does not exist yet it
 * is created, if it already exists its content is cleared before.
 *
 * The file is opened in binary mode. */
FSPP_API void
write_to_file(const path& p, const std::string& data);

}  // namespace filesystem
}  // namespace eyestep

#include "fspp/utils.ipp"
