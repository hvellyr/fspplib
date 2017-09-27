// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/path.hpp"

#include <ios>
#include <istream>
#include <memory>
#include <ostream>
#include <system_error>


namespace eyestep {
namespace filesystem {
namespace detail {
class IFileImpl;
}

/*! A representation to read from and write to real files
 *
 * While eyestep::filesystem::path represents the abstraction for a file or
 * directory pathname instances of this class are used to access the content of
 * a file.  std::iostream%s are unluckily very restricted (they can't be copied
 * or moved for instance), where this class comes in.  It implements a handle
 * interface to an std::iostream, and therefore can be copied and passed.
 * Reading and writing is done though always in terms of the std::iostream
 * system.
 *
 * @code
 * auto f = File("mfile.txt");
 * auto& is = f.open(std::ios::in | std::ios::binary);
 *
 * for (std::string line; std::getline(is, line); ) {
 *   sum += std::stoi(line);
 * }
 * f.close();
 * @endcode
 *
 * @see with_stream(), with_stream_for_reading(), and with_stream_for_writing() for an
 *      exception safe way of opening and closing files.
 *
 * @see eyestep::filesystem::directory_iterator for reading directory entries.
 */
class FSPP_API File
{
public:
  /*! Constructs an invalid file instance */
  File() = default;

  /*! Constructs a file instance for the file at @p p
   *
   * @p p does not need to exist (yet), depending on the openmode to open() the file might
   * be created later.
   */
  File(filesystem::path p);

  /*! Makes a copy of @p other
   *
   * If @p other was open at the time of the copy both instances share the same backend
   * structure, i.e. both are open and use the same stream infrastructure.  Closing on
   * instance (with close()) will close the other, too.
   */
  File(const File& other) = default;

  /*! Moves @p other into a new instance
   *
   * @p other is in an invalid state after this operation.
   *
   * When @p other was open the stream stays valid, but is only available from the new
   * instance.
   */
  File(File&& other);

  /*! Copies @p rhs into this.
   *
   * If @rhs is open it stays open and *this and @p rhs are pointing to the same file and
   * stream.  Closing one of this or @p rhs closes the stream for both.
   *
   * If the receiver (*this) was open and was the only "owner" of the stream, the stream
   * is closed as if close() was called.
   */
  File& operator=(const File& rhs) = default;
  File& operator=(File&& rhs);

  /*! Returns the path this file is refering to */
  const filesystem::path& path() const;

  /*! Indicates whether this is a valid file object.  The default c'tor returns an invalid
   * object. */
  bool is_valid() const;

  /*! Opens a file to which path() refers for @p mode and returns a reference to a
   * properly setup stream for reading and writing.  Though the stream is of type
   * std::iostream its openmode depends on @p mode.
   *
   * The stream is only available during the lifetime of the receiver (*this).  If the
   * receiver is closed (close()) the stream is implicitely reset.
   *
   * The stream returned by this method has no exception mask set, so the user of the
   * return value is responsible for checking the proper good(), fail(), eof(), and bad()
   * methods of std::iostream.
   *
   * @throws filesystem_error if any error occurs
   */
  std::iostream& open(std::ios::openmode mode);

  /*! Opens a file to which path() refers for @p mode and returns a reference to a
   * properly setup stream for reading and writing.  Though the stream is of type
   * std::iostream its openmode depends on @p mode.
   *
   * The stream is only available during the lifetime of the receiver (*this).  If the
   * receiver is closed (close()) the stream is implicitely reset.
   *
   * The stream returned by this method has no exception mask set, so the user of the
   * return value is responsible for checking the proper good(), fail(), eof(), and bad()
   * methods of std::iostream.
   *
   * If an error occurs sets @p ec accordingly.  If successfull @p ec is cleared.
   */
  std::iostream& open(std::ios::openmode mode, std::error_code& ec);

  /*! Returns a reference to the underlying stream.
   *
   * Reading from or writing to the stream if is_valid() or is_open() returns
   * false is undefined. */
  std::iostream& stream();

  /*! Indicates whether the receiver (*this) is opened. */
  bool is_open() const;

  /*! Closes the underlying stream
   *
   * It is an error to close a file which is not opened.
   *
   * @throws filesystem_error if any error occurs
   */
  void close();

  /*! Closes the underlying stream
   *
   * It is an error to close a file which is not opened.
   *
   * If an error occurs sets @p ec accordingly.  If successfull @p ec is cleared.
   */
  void close(std::error_code& ec);

private:
  std::shared_ptr<detail::IFileImpl> _impl;
  filesystem::path _path;
};

}  // namespace filesystem
}  // namespace eyestep

#include "fspp/details/file.ipp"
