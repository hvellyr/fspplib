// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/utility/bitmask_type.hpp"

#include <cstdint>
#include <ctime>

namespace eyestep {
namespace filesystem {

enum class perms
{
  /*! no permission bits are set */
  none = 0,
  /*! S_IRUSR - File owner has read permission */
  owner_read = 0400,
  /*! S_IWUSR - File owner has write permission */
  owner_write = 0200,
  /*! S_IXUSR - File owner has execute/search permission */
  owner_exec = 0100,
  /*! S_IRWXU - File owner has read, write, and execute/search permissions; Equivalent to
   * owner_read | owner_write | owner_exec */
  owner_all = 0700,
  /*! S_IRGRP - The file's user group has read permission */
  group_read = 040,
  /*! S_IWGRP - The file's user group has write permission */
  group_write = 020,
  /*! S_IXGRP - The file's user group has execute/search permission */
  group_exec = 010,
  /*! S_IRWXG - The file's user group has read, write, and execute/search permissions;
   * Equivalent to group_read | group_write | group_exec */
  group_all = 070,
  /*! S_IROTH - Other users have read permission */
  others_read = 04,
  /*! S_IWOTH - Other users have write permission */
  others_write = 02,
  /*! S_IXOTH - Other users have execute/search permission */
  others_exec = 01,
  /*! S_IRWXO - Other users have read, write, and execute/search permissions; Equivalent
   * to others_read | others_write | others_exec */
  others_all = 07,
  /*! All users have read, write, and execute/search permissions; Equivalent to owner_all
   * | group_all | others_all */
  all = 0777,
  /*! S_ISUID - Set user ID to file owner user ID on execution */
  set_uid = 04000,
  /*! S_ISGID - Set group ID to file's user group ID on execution */
  set_gid = 02000,
  /*! S_ISVTX - Implementation-defined meaning, but POSIX XSI specifies that when set on a
   * directory, only file owners may delete files even if the directory is writeable to
   * others (used with /tmp) */
  sticky_bit = 01000,
  /*! All valid permission bits.  Equivalent to all | set_uid | set_gid | sticky_bit */
  mask = 07777,
  /*! Unknown permissions (e.g. when file_status is created without permissions) */
  unknown = 0xFFFF,
  /*! Control bit that instructs permissions to add, but not clear permission bits. */
  add_perms = 0x10000,
  /*! Control bit that instructs permissions to clear, but not add permission bits */
  remove_perms = 0x20000,
  /*! Control bit that instructs permissions to resolve symlinks */
  resolve_symlinks = 0x40000,
};

FSPP_BITMASK_TYPE(perms)


/*! This type represents available options that control the behavior of the copy() and
 *  copy_file() function.
 *
 * This enum satisfies the requirements of a BitmaskType.
 *
 * In each group at most one option may be set.
 *
 * options controlling copy_file() when the file already exists:
 * - none = Report an error (default behavior)
 * - skip_existing = Keep the existing file, without reporting an error.
 * - overwrite_existing = Replace the existing file
 * - update_existing = Replace the existing file only if it is older than the file being
 *   copied
 *
 * options controlling the effects of copy() on subdirectories:
 * - none = Skip subdirectories (default behavior)
 * - recursive = Recursively copy subdirectories and their content
 *
 * options controlling the effects of copy() on symbolic links:
 * - none = Follow symlinks (default behavior)
 * - copy_symlinks = Copy symlinks as symlinks, not as the files they point to
 * - skip_symlinks = Ignore symlinks
 *
 * options controlling the kind of copying copy() does:
 * - none = Copy file content (default behavior)
 * - directories_only = Copy the directory structure, but do not copy any non-directory
 *   files
 * - create_symlinks = Instead of creating copies of files, create symlinks pointing to
 *   the originals. Note: the source path must be an absolute path unless the destination
 *   path is in the current directory.
 * - create_hard_links = Instead of creating copies of files, create hardlinks that
 *   resolve to the same files as the originals
 */
enum class copy_options
{
  none = 0,
  skip_existing = 1,
  overwrite_existing = 2,
  update_existing = 4,
  recursive = 8,
  copy_symlinks = 16,
  skip_symlinks = 32,
  directories_only = 64,
  create_symlinks = 128,
  create_hard_links = 256,
};

FSPP_BITMASK_TYPE(copy_options)


/*! This type represents available options that control the behavior of the
 *  directory_iterator and recursive_directory_iterator.
 *
 * This enum satisfies the requirements of a BitmaskType.
 *
 * - none = (Default) Skip directory symlinks, permission denied is error.
 * - follow_directory_symlink = Follow rather than skip directory symlinks.
 * - skip_permission_denied = Skip directories that would otherwise result in permission
 *   denied errors.
 */
enum class directory_options
{
  none = 0,
  follow_directory_symlink = 1,
  skip_permission_denied = 2,
};

FSPP_BITMASK_TYPE(directory_options)


/*! Represents space filesystem information */
struct space_info
{
  /*! total size of the filesystem, in bytes */
  std::uintmax_t capacity;
  /*! free space on the filesystem, in bytes */
  std::uintmax_t free;
  /*! free space available to a non-privileged process (may be equal or less than free) */
  std::uintmax_t available;
};


/*! Indicates a type of a file or directory a path refers to. */
enum class file_type
{
  /*! indicates that the file status has not been evaluated yet, or an error occurred when
   * evaluating it */
  none = 0,
  /*! indicates that the file was not found (this is not considered an error) */
  not_found = -1,
  /*! a regular file */
  regular = 1,
  /*! a directory */
  directory = 2,
  /*! a symbolic link */
  symlink = 3,
  /*! a block special file */
  block = 4,
  /*! a character special file */
  character = 5,
  /*! a FIFO (or pipe) file */
  fifo = 6,
  /*! a socket file */
  socket = 7,
  /*! an unknown file */
  unknown = 8,
};


using file_size_type = std::uintmax_t;
using file_time_type = std::time_t;

}  // namespace filesystem
}  // namespace eyestep
