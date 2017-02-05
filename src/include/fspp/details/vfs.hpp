// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/dir_iterator.hpp"
#include "fspp/filesystem.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>


namespace eyestep {
namespace filesystem {

namespace detail {
class IFileImpl;
}

namespace vfs {

/*! Interface to be implemented by virtual file system backends used by
 *  eyestep::filesystem VFS system.
 */
class IFilesystem
{
public:
  virtual ~IFilesystem() = default;

  /*! Construct a new instance of a file backend working on the IFilesystem backend. */
  virtual std::unique_ptr<detail::IFileImpl> make_file_impl() = 0;

  virtual std::unique_ptr<directory_iterator::IDirIterImpl> make_dir_iterator(
    const path& p, std::error_code& ec) = 0;

  /*! This should print the full filesystem content to @p os
   *
   * The format is up to the implementation, but folder and file structure, attributes
   * like file size and modification dates should be visible.  Its sole purpose is to
   * debug the internals of the VFS during development.
   */
  virtual void dump(std::ostream& os) = 0;

  virtual path canonical(const path& p, const path& base, std::error_code& ec) = 0;

  virtual bool copy_file(const path& from,
                         const path& to,
                         copy_options options,
                         std::error_code& ec) = 0;

  virtual void copy_symlink(const path& from, const path& to, std::error_code& ec) = 0;

  /*! Creates the directory @p p as if by POSIX mkdir() with a second argument of
   *  static_cast<int>(fs::perms::all) (the parent directory must already exist).  If @p p
   *  already exists and is already a directory, the function does nothing (this condition
   *  is not treated as an error).
   *
   * @returns @c true if directory creation is successful, @c false otherwise.
   */
  virtual bool create_directory(const path& p, std::error_code& ec) = 0;

  /*! Same as create_directory(const path&), except that the attributes of the new
   * directory are copied from @p existing_p (which must be a directory that exists).  It
   * is OS-dependent which attributes are copied.
   *
   * @returns @c true if directory creation is successful, @c false otherwise.
   */
  virtual bool create_directory(const path& p,
                                const path& existing_p,
                                std::error_code& ec) = 0;

  /*! Executes create_directory(const path&) for every element of p that does not already
   * exist.
   *
   * @returns @c true if directory creation is successful, @c false otherwise.
   */
  virtual bool create_directories(const path& p, std::error_code& ec) = 0;

  virtual void create_hard_link(const path& target,
                                const path& link,
                                std::error_code& ec) = 0;

  virtual void create_symlink(const path& target,
                              const path& link,
                              std::error_code& ec) = 0;
  virtual void create_directory_symlink(const path& target,
                                        const path& link,
                                        std::error_code& ec) = 0;

  virtual bool equivalent(const path& p1, const path& p2, std::error_code& ec) = 0;

  virtual file_size_type file_size(const path& p, std::error_code& ec) = 0;

  virtual std::uintmax_t hard_link_count(const path& p, std::error_code& ec) = 0;

  virtual file_time_type last_write_time(const path& p, std::error_code& ec) = 0;
  virtual void last_write_time(const path& p,
                               file_time_type new_time,
                               std::error_code& ec) = 0;

  virtual void permissions(const path& p, perms prms, std::error_code& ec) = 0;

  virtual path read_symlink(const path& p, std::error_code& ec) = 0;

  virtual bool remove(const path& p, std::error_code& ec) = 0;
  virtual std::uintmax_t remove_all(const path& p, std::error_code& ec) = 0;

  virtual void rename(const path& old_p, const path& new_p, std::error_code& ec) = 0;

  virtual void resize_file(const path& p,
                           file_size_type new_size,
                           std::error_code& ec) = 0;

  virtual space_info space(const path& p, std::error_code& ec) NOEXCEPT = 0;
  virtual file_status status(const path& p, std::error_code& ec) NOEXCEPT = 0;

  virtual file_status symlink_status(const path& p, std::error_code& ec) NOEXCEPT = 0;
};

/*! Create a new memory filesystem.  The filesystem is empty except for the root directory
 *  ("/"). */
FSPP_API std::unique_ptr<IFilesystem>
make_memory_filesystem();


/*! Registers a virtual filesystem @p fs for the root name @p name.
 *
 * @p name has the exact form as it would be returned from path::root_name(),
 * e.g. "//<vfs>".  To avoid name clashes with real existing UNC notated path the function
 * only accepts name which begin with "//<".
 */
FSPP_API void
register_vfs(const std::string& name, std::unique_ptr<IFilesystem> fs);
/*! Unregisters a filesystem registered with the root name @p name.
 *
 * @returns the filesystem instance after unregistration.
 */
FSPP_API std::unique_ptr<IFilesystem>
unregister_vfs(const std::string& name);

/* Constructs a new memory virtual file system, registers it for @p name, and calls @p
 * functor with it.  When @p functor returns the file system is unregistered and
 * destroyed.
 *
 * The signature of @p functor should be equivalent to `void fun(IFilesystem&)`.
 *
 * @p name has the exact form as it would be returned from `path::root_name()`.
 *
 * @code
 * with_memory_vfs("//<vfs>",
 *                [](IFilesystem& fs) {
 *                  ...
 *                });
 * @endcode
 */
template <typename Functor>
void
with_memory_vfs(const std::string& name, Functor functor);

}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep

#include "fspp/details/vfs.ipp"
