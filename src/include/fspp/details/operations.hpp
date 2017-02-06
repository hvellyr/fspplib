// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/path.hpp"
#include "fspp/details/platform.hpp"
#include "fspp/details/types.hpp"

#include <system_error>


namespace eyestep {
namespace filesystem {

/*! Returns the current path, obtained as if by POSIX getcwd.
 *
 * @note This operation doesn't work with the virtual filesystem.
 */
FSPP_API path
current_path();

/*! Copies the file or directory @p from to file or directory @p to, using the copy
 * options indicated by @p options (defaults to copy_options::none for the versions not
 * taking it).
 *
 * The behavior is undefined if there is more than one option in any of the copy_options
 * option group present in options (even in the copy_file group, which is not relevant to
 * copy) */
FSPP_API void
copy(const path& from, const path& to);
FSPP_API void
copy(const path& from, const path& to, std::error_code& ec) NOEXCEPT;
FSPP_API void
copy(const path& from, const path& to, copy_options options);
FSPP_API void
copy(const path& from,
     const path& to,
     copy_options options,
     std::error_code& ec) NOEXCEPT;

/*! Copies a single file from @p from to @p to, using the copy options indicated by @p
 *  options (which are `copy_options::none` in the variant without @p options parameter).
 *  The behavior is undefined if there is more than one option in any of the copy_options
 *  option group present in options (even in the groups not relevant to copy_file).
 */
FSPP_API bool
copy_file(const path& from, const path& to);
FSPP_API bool
copy_file(const path& from, const path& to, std::error_code& ec) NOEXCEPT;
FSPP_API bool
copy_file(const path& from, const path& to, copy_options options);
FSPP_API bool
copy_file(const path& from,
          const path& to,
          copy_options options,
          std::error_code& ec) NOEXCEPT;

/*! Copies a symlink to another location. */
FSPP_API void
copy_symlink(const path& from, const path& to);
FSPP_API void
copy_symlink(const path& from, const path& to, std::error_code& ec) NOEXCEPT;

/*! Creates the directory @p p as if by POSIX `mkdir()` with a second argument of
 * static_cast<int>(std::filesystem::perms::all).
 *
 * The parent directory must already exist.  If @p p already exists and is already a
 * directory, the function does nothing (this condition is not treated as an error).
 *
 * @returns true if directory creation is successful, false otherwise. */
FSPP_API bool
create_directory(const path& p);
FSPP_API bool
create_directory(const path& p, std::error_code& ec) NOEXCEPT;

/*! Same as create_directory(path), except that the attributes of the new directory are
 *  copied from @p existing_p (which must be a directory that exists).
 *
 * It is OS-dependent which attributes are copied.
 *
 * @returns true if directory creation is successful, false otherwise. */
FSPP_API bool
create_directory(const path& p, const path& existing_p);
FSPP_API bool
create_directory(const path& p, const path& existing_p, std::error_code& ec) NOEXCEPT;

/*! Executes create_directory(path) for every element of @p p that does not already
 * exist.
 *
 * @returns true if directory creation is successful, false otherwise. */
FSPP_API bool
create_directories(const path& p);
FSPP_API bool
create_directories(const path& p, std::error_code& ec) NOEXCEPT;

/*! Creates a hard link link with its target set to target as if by POSIX `link()`: the
 *  pathname target must exist.
 *
 * Once created, link and target are two logical names that refer to the same file (they
 * are equivalent).  Even if the original name target is deleted, the file continues to
 * exist and is accessible as link.  */
FSPP_API void
create_hard_link(const path& target, const path& link);
FSPP_API void
create_hard_link(const path& target, const path& link, std::error_code& ec) NOEXCEPT;

/*! Creates a symbolic link @p link with its target set to @p target as if by POSIX
 *  symlink(): the pathname target may be invalid or non-existing. */
FSPP_API void
create_symlink(const path& target, const path& link);
FSPP_API void
create_symlink(const path& target, const path& link, std::error_code& ec) NOEXCEPT;
/*! Creates a symbolic link link with its target set to target as if by POSIX symlink():
 *  the pathname target may be invalid or non-existing.
 *
 * Some operating systems require symlink creation to identify that the link is to a
 * directory. Portable code should use his function to create directory symlinks rather
 * than create_symlink(), even though there is no distinction on POSIX systems. */
FSPP_API void
create_directory_symlink(const path& target, const path& link);
FSPP_API void
create_directory_symlink(const path& target,
                         const path& link,
                         std::error_code& ec) NOEXCEPT;

/*! Returns the current path, obtained as if by POSIX getcwd.  Returns path() if error
 * occurs. */
FSPP_API path
current_path(std::error_code& ec) NOEXCEPT;
/*! Changes the current path to @p p, as if by POSIX @c chdir. */
FSPP_API void
current_path(const path& p);
/*! Changes the current path to @p p, as if by POSIX @c chdir. */
FSPP_API void
current_path(const path& p, std::error_code& ec) NOEXCEPT;

/*! Checks whether the paths @p p1 and @p p2 refer to the same file or directory and have
 *  the same file status as determined by status() (symlinks are followed).
 *
 * If @p p1 or @p p2 does not exist or if their file type is not file, directory, or
 * symlink (as determined by is_other()), an error is reported.
 *
 * The non-throwing overload returns false on errors. */
FSPP_API bool
equivalent(const path& p1, const path& p2);
FSPP_API bool
equivalent(const path& p1, const path& p2, std::error_code& ec) NOEXCEPT;

/*! Checks if the given file status or path corresponds to an existing file or directory.
 *
 * Equivalent to <tt>status_known(s) && s.type() != file_type::not_found</tt>.
 */
FSPP_API bool
exists(file_status s) NOEXCEPT;

/*! Checks if the given file status or path corresponds to an existing file or directory.
 *
 * Equivalent to exists(status(p)) or exists(status(p, ec)) (symlinks are followed). The
 * non-throwing overload returns false if an error occurs.
 *
 * The information of this function is also provided by directory iteration.
 */
FSPP_API bool
exists(const path& p);
FSPP_API bool
exists(const path& p, std::error_code& ec) NOEXCEPT;

/*! Returns the size of the regular file @p p, determined as if by reading the `st_size`
 *  member of the structure obtained by POSIX `stat()` (symlinks are followed).
 *
 * Attempting to determine the size of a directory (as well as any other file that is not
 * a regular file or a symlink) is treated as an error.
 *
 * The non-throwing overload returns returns -1 on errors. */
FSPP_API file_size_type
file_size(const path& p);
FSPP_API file_size_type
file_size(const path& p, std::error_code& ec) NOEXCEPT;

/* Returns the number of hard links for the filesystem object identified by path p. */
FSPP_API std::uintmax_t
hard_link_count(const path& p);
/*! Returns the number of hard links for the filesystem object identified by path @p p.
 *
 * @returns `static_cast<uintmax_t>(-1)` on errors. */
FSPP_API std::uintmax_t
hard_link_count(const path& p, std::error_code& ec) NOEXCEPT;

/*! Returns the time of the last modification of p, determined as if by accessing the
 *  member st_mtime of the POSIX stat (symlinks are followed) The non-throwing overload
 *  returns file_time_type::min() on errors. */
FSPP_API file_time_type
last_write_time(const path& p);
FSPP_API file_time_type
last_write_time(const path& p, std::error_code& ec) NOEXCEPT;
/*! Changes the time of the last modification of p, as if by POSIX futimens (symlinks are
 *  followed) */
FSPP_API void
last_write_time(const path& p, file_time_type new_time);
FSPP_API void
last_write_time(const path& p, file_time_type new_time, std::error_code& ec) NOEXCEPT;

/*! Changes access permissions of the file to which @p p resolves, as if by POSIX
 *  `fchmodat()`.  Symlinks are followed if `prms::resolve_symlinks` is set. */
FSPP_API void
permissions(const path& p, perms prms);
FSPP_API void
permissions(const path& p, perms prms, std::error_code& ec) NOEXCEPT;

/*! If the path @p p refers to a symbolic link, returns a new path object which refers to
 *  the target of that symbolic link.
 *
 * It is an error if @p p does not refer to a symbolic link.
 *
 * @returns The non-throwing overload returns an empty path on errors. */
FSPP_API path
read_symlink(const path& p);
FSPP_API path
read_symlink(const path& p, std::error_code& ec) NOEXCEPT;

/*! The file or empty directory identified by the path @p p is deleted as if by the POSIX
 * `remove`.
 *
 * Symlinks are not followed (symlink is removed, not its target) */
FSPP_API bool
remove(const path& p);
FSPP_API bool
remove(const path& p, std::error_code& ec) NOEXCEPT;

/*! Deletes the contents of @p p (if it is a directory) and the contents of all its
 *  subdirectories, recursively, then deletes @p p itself as if by repeatedly applying the
 *  POSIX call `remove`.
 *
 * Symlinks are not followed (symlink is removed, not its target) */
FSPP_API std::uintmax_t
remove_all(const path& p);
FSPP_API std::uintmax_t
remove_all(const path& p, std::error_code& ec) NOEXCEPT;

/*! Moves or renames the filesystem object identified by @p old_p to @p new_p as if by the
 *  POSIX `rename()`. */
FSPP_API void
rename(const path& old_p, const path& new_p);
FSPP_API void
rename(const path& old_p, const path& new_p, std::error_code& ec) NOEXCEPT;

/*! Changes the size of the regular file named by @p p as if by POSIX `truncate()`.
 *
 * If the file size was previously larger than @p new_size, the remainder of the file is
 * discarded.  If the file was previously smaller than @p new_size, the file size is
 * increased and the new area appears as if zero-filled. */
FSPP_API void
resize_file(const path& p, file_size_type new_size);
FSPP_API void
resize_file(const path& p, file_size_type new_size, std::error_code& ec) NOEXCEPT;

/*! Determines the information about the filesystem on which the pathname @p p is located,
 *  as if by POSIX `statvfs()`.
 *
 * Populates and returns an object of type space_info, set from the members of the POSIX
 * struct statvfs as follows:
 *
 * - `space_info.capacity` is set as if by `f_blocks * f_frsize`
 * - `space_info.free` is set to `f_bfree * f_frsize`
 * - `space_info.available` is set to `f_bavail * f_frsize`
 *
 * Any member that could not be determined is set to `static_cast<uintmax_t>(-1)`.
 *
 * @returns The non-throwing overload sets all members to `static_cast<uintmax_t>(-1)` on
 * error. */
FSPP_API space_info
space(const path& p);
FSPP_API space_info
space(const path& p, std::error_code& ec) NOEXCEPT;

/*! Determines the type and attributes of the filesystem object identified by p as if by
 *  POSIX stat (symlinks are followed to their targets). */
FSPP_API file_status
status(const path& p);
FSPP_API file_status
status(const path& p, std::error_code& ec) NOEXCEPT;

/*! Same as status() except that the behavior is as if the POSIX `lstat()` is used
 *  (symlinks are not followed).
 *
 * If @p p is a symlink, returns file_status(file_type::symlink). */
FSPP_API file_status
symlink_status(const path& p);
FSPP_API file_status
symlink_status(const path& p, std::error_code& ec) NOEXCEPT;

/*! Returns the directory location suitable for temporary files.  The path is guaranteed
  to exist and to be a directory. */
FSPP_API path
temp_directory_path();
FSPP_API path
temp_directory_path(std::error_code& ec) NOEXCEPT;


/*! Updates the last modification time of path @p p
 *
 * This is like <tt>void last_write_time()</tt> except that when @p p does not yet exist
 * it is created with a length of 0 bytes.
 *
 * Extension to ISO/IEC TS 18822:2015.
 */
FSPP_API void
touch(const path& p);
FSPP_API void
touch(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks if the given file status or path corresponds to a block special file, as if
 *  determined by the POSIX S_ISBLK . Examples of block special files are block devices
 *  such as /dev/sda or /dev/loop0 on Linux. */
FSPP_API bool
is_block_file(file_status s) NOEXCEPT;
FSPP_API bool
is_block_file(const path& p);
FSPP_API bool
is_block_file(const path& p, std::error_code& ec) NOEXCEPT;

/*! Checks if the given file status or path corresponds to a character special file, as if
 *  determined by POSIX S_ISCHR. Examples of character special files are character devices
 *  such as /dev/null, /dev/tty, /dev/audio, or /dev/nvram on Linux. */
FSPP_API bool
is_character_file(file_status s) NOEXCEPT;
FSPP_API bool
is_character_file(const path& p);
FSPP_API bool
is_character_file(const path& p, std::error_code& ec) NOEXCEPT;

/*! Checks if the given file status or path corresponds to a directory. */
FSPP_API bool
is_directory(file_status s) NOEXCEPT;
FSPP_API bool
is_directory(const path& p);
FSPP_API bool
is_directory(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks whether the given path refers to an empty file or directory. */
FSPP_API bool
is_empty(const path& p);
FSPP_API bool
is_empty(const path& p, std::error_code& ec);


/*! Checks if the given file status or path corresponds to a FIFO or pipe file as if
 *  determined by POSIX S_ISFIFO. */
FSPP_API bool
is_fifo(file_status s) NOEXCEPT;
FSPP_API bool
is_fifo(const path& p);
FSPP_API bool
is_fifo(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks if the given file status or path corresponds to a file of type other type. That
 *  is, the file exists, but is neither regular file, nor directory nor a symlink.
 *
 * - Equivalent to <tt>exists(s) && !is_regular_file(s) && !is_directory(s) &&
 *   !is_symlink(s).</tt>
 * - Equivalent to <tt>is_other(status(p))</tt> or <tt>is_other(status(p, ec))</tt>,
 *   respectively.
 */
FSPP_API bool
is_other(file_status s);
FSPP_API bool
is_other(const path& p);
FSPP_API bool
is_other(const path& p, std::error_code& ec);


/*! Checks if the given file status or path corresponds to a regular file. */
FSPP_API bool
is_regular_file(file_status s) NOEXCEPT;
FSPP_API bool
is_regular_file(const path& p);
FSPP_API bool
is_regular_file(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks if the given file status or path corresponds to a named IPC socket, as if
 *  determined by the POSIX S_IFSOCK */
FSPP_API bool
is_socket(file_status s) NOEXCEPT;
FSPP_API bool
is_socket(const path& p);
FSPP_API bool
is_socket(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks if the given file status or path corresponds to a symbolic link, as if
 *  determined by the POSIX S_IFLNK.*/
FSPP_API bool
is_symlink(file_status s) NOEXCEPT;
FSPP_API bool
is_symlink(const path& p);
FSPP_API bool
is_symlink(const path& p, std::error_code& ec) NOEXCEPT;


/*! Checks if the given file status is known, Equivalent to <tt>s.type() ==
 *  file_type::none</tt>. */
FSPP_API bool
status_known(file_status s) NOEXCEPT;

}  // namespace filesystem
}  // namespace eyestep
