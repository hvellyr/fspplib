// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/path.hpp"
#include "fspp/details/types.hpp"
#include "fspp/estd/optional.hpp"

#include <iterator>
#include <memory>
#include <system_error>
#include <utility>
#include <vector>


namespace eyestep {
namespace filesystem {

/*! Represents a directory entry. The object stores a path as a member. */
class FSPP_API directory_entry
{
public:
  directory_entry() = default;
  directory_entry(const directory_entry& other) = default;
#if defined(FSPP_IS_VS2013)
  directory_entry(directory_entry&& other);
#else
  directory_entry(directory_entry&& other) = default;
#endif

  explicit directory_entry(const filesystem::path& p);

  directory_entry& operator=(const directory_entry& other) = default;
#if defined(FSPP_IS_VS2013)
  directory_entry& operator=(directory_entry&& other);
#else
  directory_entry& operator=(directory_entry&& other) = default;
#endif

  /*! Returns the full path the directory entry refers to. */
  const filesystem::path& path() const;
  /*! Returns the full path the directory entry refers to. */
  operator const filesystem::path&() const;

  /*! Returns the status of the pointed to path, as if determined by a status() call
   * (i.e. symlinks are followed). */
  file_status status() const;
  /*! Returns the status of the pointed to path, as if determined by a status() call
   * (i.e. symlinks are followed). */
  file_status status(std::error_code& ec) const;

  /*! Returns the status of the pointed to path, as if determined by a symlink_status()
   * call (i.e. symlinks are not followed). */
  file_status symlink_status() const;
  /*! Returns the status of the pointed to path, as if determined by a symlink_status()
   * call (i.e. symlinks are not followed). */
  file_status symlink_status(std::error_code& ec) const;

  /*! Returns the file_size of this entry
   *
   * @returns if *this contains a cached file size, return it. Otherwise return
   * file_size(path()) or file_size(path(), ec) respectively.
   *
   * @throws As specified in Error reporting (7). */
  file_size_type file_size() const;
  file_size_type file_size(std::error_code& ec) const NOEXCEPT;

  /*! Assigns new content to the directory entry object. Sets the path to p. */
  void assign(const filesystem::path& p);

  void assign(const filesystem::path& p, file_size_type file_size);

  /*! Compares the path with the directory entry rhs. */
  friend bool operator==(const directory_entry& lhs, const directory_entry& rhs);
  friend bool operator!=(const directory_entry& lhs, const directory_entry& rhs);
  friend bool operator<(const directory_entry& lhs, const directory_entry& rhs);
  friend bool operator<=(const directory_entry& lhs, const directory_entry& rhs);
  friend bool operator>(const directory_entry& lhs, const directory_entry& rhs);
  friend bool operator>=(const directory_entry& lhs, const directory_entry& rhs);

private:
  filesystem::path _path;
  estd::optional<file_size_type> _file_size;
};


/*! directory_iterator is an InputIterator that iterates over the directory_entry
 *  elements of a directory (but does not visit the subdirectories). The iteration order
 *  is unspecified, except that each directory entry is visited only once. The special
 *  pathnames dot and dot-dot are skipped.
 *
 * If the directory_iterator is advanced past the last directory entry, it becomes equal
 * to the default-constructed iterator, also known as the end iterator. Two end
 * iterators are always equal, dereferencing or incrementing the end iterator is
 * undefined behavior.
 *
 * If a file or a directory is deleted or added to the directory tree after the
 * directory iterator has been created, it is unspecified whether the change would be
 * observed through the iterator. */
class FSPP_API directory_iterator
{
public:
  class IDirIterImpl;

  using value_type = directory_entry;
  using difference_type = std::ptrdiff_t;
  using pointer = const directory_entry*;
  using reference = const directory_entry&;
  using iterator_category = std::input_iterator_tag;

  /*! Constructs a new directory iterator. */

  /*! Constructs the end iterator. */
  directory_iterator() NOEXCEPT;

  ~directory_iterator();

  /*! Constructs a directory iterator that refers to the first directory entry of a
   *  directory identified by @p p.
   *
   * If @p p refers to an non-existing file or not a directory, returns the end iterator.
   */
  explicit directory_iterator(const path& p);
  directory_iterator(const path& p, std::error_code& ec) NOEXCEPT;

  directory_iterator(const directory_iterator&) = default;
#if defined(FSPP_IS_VS2013)
  directory_iterator(directory_iterator&&);
#else
  directory_iterator(directory_iterator&&) = default;
#endif


  directory_iterator& operator=(const directory_iterator&) = default;
#if defined(FSPP_IS_VS2013)
  directory_iterator& operator=(directory_iterator&&);
#else
  directory_iterator& operator=(directory_iterator&&) = default;
#endif

  /*! Accesses the pointed-to directory_entry.
   *
   * The result of operator* or operator-> on the end iterator is undefined behavior. */
  reference operator*() const;
  pointer operator->() const;

  /*! Advances the iterator to the next entry.
   *
   * In case of an error this operator throws a filesystem_error exception. */
  directory_iterator& operator++();
  /*! Advances the iterator to the next entry.
   *
   * In case of an error @p ec is set.  If successful @p ec is cleared. */
  directory_iterator& increment(std::error_code& ec) NOEXCEPT;

  bool operator==(const directory_iterator& rhs) const;
  bool operator!=(const directory_iterator& rhs) const;

private:
  std::shared_ptr<IDirIterImpl> _impl;
};


/*! Returns iter unchanged
 *
 * These non-member function (together with the non-member end()) enables the use of
 * directory_iterator%s with range-based for loops. */
directory_iterator
begin(directory_iterator iter);

/*! Returns a default-constructed directory_iterator, which serves as the end
 * iterator. The argument is ignored.
 *
 * These non-member function (together with the non-member begin()) enables the use of
 * directory_iterator%s with range-based for loops. */
directory_iterator
end(const directory_iterator&);


/*! recursive_directory_iterator is an InputIterator that iterates over the
 *  directory_entry elements of a directory, and, recursively, over the entries of all
 *  subdirectories. The iteration order is unspecified, except that each directory entry
 *  is visited only once.
 *
 * By default, symlinks are not followed, but this can be enabled by specifying the
 * directory option follow_directory_symlink at construction time.
 *
 * The special pathnames dot and dot-dot are skipped.
 *
 * If the recursive_directory_iterator is advanced past the last directory entry of the
 * top-level directory, it becomes equal to the default-constructed iterator, also known
 * as the end iterator. Two end iterators are always equal, dereferencing or incrementing
 * the end iterator is undefined behavior.
 *
 * If a file or a directory is deleted or added to the directory tree after the recursive
 * directory iterator has been created, it is unspecified whether the change would be
 * observed through the iterator.
 *
 * If the directory structure contains cycles, the end iterator may be unreachable. */
class FSPP_API recursive_directory_iterator
{
public:
  class IImpl;

  using value_type = directory_entry;
  using difference_type = std::ptrdiff_t;
  using pointer = const directory_entry*;
  using reference = const directory_entry&;
  using iterator_category = std::input_iterator_tag;

  /*! Default constructor. Constructs an end iterator. */
  recursive_directory_iterator() NOEXCEPT;
  /*! Contructs new recursive directory iterator. */
  recursive_directory_iterator(const recursive_directory_iterator&) = default;
#if defined(FSPP_IS_VS2013)
  recursive_directory_iterator(recursive_directory_iterator&&);
#else
  recursive_directory_iterator(recursive_directory_iterator&&) = default;
#endif

  /*! Contructs a new recursive directory iterator that refers to the first entry in the
   * directory that p resolves to.
   *
   * Recursive directory iterators do not follow directory symlinks by default. To enable
   * this behavior, specify directory_options::follow_directory_symlink among the options
   * option set. */
  explicit recursive_directory_iterator(
    const path& p, directory_options options = directory_options::none);
  recursive_directory_iterator(const path& p,
                               directory_options options,
                               std::error_code& ec) NOEXCEPT;
  recursive_directory_iterator(const path& p, std::error_code& ec) NOEXCEPT;

  recursive_directory_iterator& operator=(const recursive_directory_iterator&) = default;
#if defined(FSPP_IS_VS2013)
  recursive_directory_iterator& operator=(recursive_directory_iterator&&);
#else
  recursive_directory_iterator& operator=(recursive_directory_iterator&&) = default;
#endif

  /*! Advances the iterator to the next entry. */
  recursive_directory_iterator& operator++();
  recursive_directory_iterator& increment(std::error_code& ec) NOEXCEPT;

  /*! Moves the iterator one level up in the directory hierarchy.
   *
   * If the parent directory is outside directory hierarchy that is iterated on
   * `(i.e. depth() == 0)`, sets *this to an end directory iterator. */
  void pop();

  /*! Accesses the pointed-to directory_entry
   *
   * The result of operator* or operator-> on the end iterator is undefined behavior. */
  const directory_entry& operator*() const;
  const directory_entry* operator->() const;

  /*! Returns the options that affect the directory iteration. The options can only be
   *  supplied when constructing the directory iterator.
   *
   * If the options argument was not supplied, returns options::none. */
  directory_options options() const NOEXCEPT;

  /*! Returns the number of directories from the starting directory to the currently
   *  iterated directory, i.e. the current depth of the directory hierarchy.
   *
   * The starting directory has depth of 0, its subdirectories have depth 1, etc.
   *
   * The behavior is undefined if *this is the end iterator. */
  int depth() const;

  /*! Returns true if the next increment will cause the directory currently referred to by
   *  *this to be iterated into.
   *
   * This function returns true immediately after construction or an increment. Recursion
   * can be disabled via disable_recursion_pending(). */
  bool recursion_pending() const NOEXCEPT;

  /*! Disables recursion to the currently referred subdirectory, if any.
   *
   * The call modifies the pending recursion flag on the iterator in such a way that the
   * next time increment is called, the iterator will advance within the current directly
   * even if it is currently referring to a subdirectory that hasn't been visited.
   *
   * The status of the pending recursion flag can be queried with recursion_pending(),
   * which is false after this call. It is reset back to true after increment, and its
   * initial value is also true.
   *
   * The behavior is undefined if *this is the end iterator. */
  void disable_recursion_pending();

  bool operator==(const recursive_directory_iterator& rhs) const;
  bool operator!=(const recursive_directory_iterator& rhs) const;

private:
  std::shared_ptr<IImpl> _impl;
};


/*! Returns iter unchanged
 *
 * These non-member function (together with the non-member end()) enables the use of
 * recursive_directory_iterator%s with range-based for loops. */
recursive_directory_iterator
begin(recursive_directory_iterator iter);

/*! Returns a default-constructed recursive_directory_iterator, which serves as the end
 * iterator. The argument is ignored.
 *
 * These non-member function (together with the non-member begin()) enables the use of
 * recursive_directory_iterator%s with range-based for loops. */
recursive_directory_iterator
end(const recursive_directory_iterator&);

}  // namespace filesystem
}  // namespace eyestep

#include "fspp/details/dir_iterator.ipp"
