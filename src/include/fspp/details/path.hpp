// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/path_convert.hpp"
#include "fspp/details/platform.hpp"

#include <string>
#include <type_traits>
#include <utility>


namespace eyestep {
namespace filesystem {

/*! Objects of type path represent paths on a filesystem.
 *
 * Only syntactic aspects of paths are handled: the pathname may represent a non-existing
 * path or even one that is not allowed to exist on the current file system or OS.
 *
 * This is mostly an implementation of std::experimental::filesystem::path from ISO/IEC TS
 * 18822:2015.  The biggest difference is the handling of 8bit encoded strings.  In the
 * standard 8bit encoded strings in the constructor, in concat(), etc. are expected to be
 * in platform encoding, i.e. in the "native narrow encoding".  For Mac OS X this is
 * (normally) UTF8; for Windows this is whatever the code page is set for the user
 * account.
 *
 * Since this is expected to be a major error case in cross platform code, this
 * implementation expects all 8bit strings (std::string or char*) to be UTF8 encoded.
 * This means on windows platform that 8bit strings are converted from UTF8 to UTF16
 * before used.  Member methods like string() or generic_string() are consequently
 * implemented in this way.  The non-member function u8path() is available for
 * compatibility, but is simple wrapper.
 */
class FSPP_API path
{
public:
/*! Character type used by the native encoding of the filesystem: char on POSIX, wchar_t
 * on Windows */
#if defined(FSPP_IS_WIN)
  using value_type = wchar_t;
#elif defined(FSPP_IS_MAC) || defined(FSPP_IS_UNIX)
  using value_type = char;
#else
#error Unsupported OS
#endif

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  CONSTEXPR static const value_type preferred_separator = '\\';
#elif defined(FSPP_IS_MAC) || defined(FSPP_IS_UNIX)
  CONSTEXPR static const value_type preferred_separator = '/';
#else
#error Unsupported OS
#endif

  using string_type = std::basic_string<value_type>;
  class iterator;
  using const_iterator = path::iterator;

private:
  string_type _data;

public:
  /*! Constructs a new path object. */
  path() = default;

  /*! Copy constructor. Constructs a copy of p. */
  path(const path&) = default;

/*! Move constructor. Constructs a copy of p, p is left in valid but unspecified
 * state. */
#if defined(FSPP_IS_VS2013)
  path(path&& other);
#else
  path(path&& other) = default;
#endif

  /*! Constructs the path from a character sequence provided by source, which is a pointer
   *  or an input iterator to a null-terminated character/wide character sequence or an
   *  std::basic_string, or represented as a pair of input iterators [first, last).
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   */
  path(const std::string& source);
  path(const char* source);
#if defined(FSPP_SUPPORT_WSTRING_API)
  path(const std::wstring& source);
  path(const wchar_t* source);
#endif

  template <typename Source>
  explicit path(const Source& source);

  /*! Constructs the path from a character sequence represented by the two iterators @p
   *  i_first and @p i_last.
   */
  template <class InputIt>
  path(InputIt i_first, InputIt i_last);

  /*! Replaces the contents of *this with a copy of the contents of p. */
  path& operator=(const path& other) = default;

/*! Replaces the contents of *this with p, possibly using move semantics: p is left in
 *  valid, but unspecified state.
 *
 * @returns *this.
 */
#if defined(FSPP_IS_VS2013)
  path& operator=(path&& other);
#else
  path& operator=(path&& other) = default;
#endif

  /*! Replaces the contents of *this with a new path value constructed from source as by
   *  the templated path constructor.  Equivalent to assign(source).
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   *
   * @returns *this.
   */
  template <typename Source>
  path& operator=(const Source& source);

  /*! Assigns the contents of @p source to the path object.
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   *
   * @returns *this.
   */
  template <typename Source>
  path& assign(const Source& source);

  /*! Assigns the contents presented by the two iterators @p i_first and @p i_last to the
   *  path object.
   *
   * @returns *this.
   */
  template <typename InputIt>
  path& assign(InputIt i_first, InputIt i_last);

  /*! First, appends the preferred directory separator to this, then, appends p.native()
   * to the pathname maintained by @p *this .
   *
   * The directory separator is not appended if the separator would be redundant, the
   * separator would make *this an absolute path, @p p is empty, or @p p.native() begins
   * with a separator.
   *
   * @returns *this.
   */
  path& operator/=(const path& p);

  /*! First, appends the preferred directory separator to this, then, appends source.
   *
   * Equivalent to <tt>operator/=(path(source))</tt>
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   *
   * @returns *this.
   */
  template <class Source>
  path& operator/=(const Source& source);

  /*! First, appends the preferred directory separator to this, then, appends source. to
   * the pathname maintained by @p *this .
   *
   * Equivalent to <tt>operator/=(path(source))</tt>
   *
   * @returns *this.
   */
  template <class Source>
  path& append(const Source& source);

  /*! First, appends the preferred directory separator to this, then, appends the string
   * represented by the two iterators @p i_first and @p i_last to.
   *
   * @returns *this.
   */
  template <typename InputIt>
  path& append(InputIt i_first, InputIt i_last);

  /*! Concatenates @p *this and @p p in such a way that native() of the result is exactly
   * original native() concatenated with p.native().
   *
   * @returns *this.
   */
  path& operator+=(const path& p);
  path& operator+=(const string_type& str);
  path& operator+=(const value_type* ptr);

  /*! Concats @p c to the native() of the receiver.
   *
   * @returns *this.
   */
  template <typename CharT,
            typename std::enable_if<std::is_integral<CharT>::value>::type* = nullptr>
  path& operator+=(CharT c);

  /*! Concatenates @p *this and @p source in such a way that native() of the result is
   * exactly original native() concatenated with path(source)
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   *
   * @returns *this.
   */
  template <class Source,
            typename std::enable_if<!std::is_integral<Source>::value>::type* = nullptr>
  path& operator+=(const Source& source);

  /*! Concatenates @p *this and @p source in such a way that native() of the result is
   * exactly original native() concatenated with path(source)
   *
   * Equivalent to <tt>operator+=(path(source))</tt>.
   *
   * @note in difference to ISO/IEC TS 18822:2015 if @p source is an 8bit string it @em
   *   must be utf8 encoded.
   *
   * @returns *this.
   */
  template <class Source>
  path& concat(const Source& source);

  /*! Clears the stored pathname. */
  void clear();

  /*! Indicates whether the path is empty. */
  bool empty() const;

  /*! Converts all directory separators in path to the preferred directory separator.
   *
   * For example, on systems, where `\` is the preferred separator, the path `foo/bar`
   * will be converted to foo\\bar.
   *
   * @returns *this
   */
  path& make_preferred();

  /*! Removes a single filename component.
   *
   * The behavior is undefined if the path has no filename component (has_filename()
   * returns false).  Otherwise is equivalent to <tt>*this = parent_path()</tt>.
   *
   * @returns *this
   */
  path& remove_filename();

  /*! Replaces a single filename component with replacement.
   *
   * The behavior is undefined if the path has no filename component (has_filename()
   * returns false).
   *
   * @returns *this
   */
  path& replace_filename(const path& replacement);

  /*! Replaces the extension with @p replacement or removes it when the default value of
   * replacement is used.
   *
   * Firstly, if this path has an extension(), it is removed.  Then, a dot character is
   * appended if @p replacement is not empty and does not begin with a dot character.
   * Then @p replacement is appended to the path.
   *
   * @returns this
   */
  path& replace_extension(const path& replacement = path());

  /*! Swaps the contents of *this and other. */
  void swap(path& other);

  /*! Returns the native string representation as null-terminated native character
   * array.
   *
   * Equivalent to <tt>native().c_str()</tt>.
   *
   * This string is suitable for use with OS APIs.
   */
  const value_type* c_str() const;

  /*! Returns the native string representation of the pathname by reference.
   *
   * This string is suitable for use with OS APIs.
   */
  const string_type& native() const;

  /*! Returns the native string representation of the pathname by value.
   *
   * This string is suitable for use with OS APIs.
   */
  operator string_type() const;

  /*! Returns the path to UTF-8 encoded string in native format
   *
   * @note This is a difference to ISO/IEC TS 18822:2015 and is equivalent to u8string()
   *       in the standard.
   */
  std::string string() const;

  /*! Returns the path to UTF-8 encoded string in native format
   *
   * The encoding is always UTF-8.
   */
  std::string u8string() const;

  /*! Returns the path to UTF-8 encoded string in generic format
   *
   * @note This is a difference to ISO/IEC TS 18822:2015 and is equivalent to
   *       generic_u8string() in the standard.
   */
  std::string generic_string() const;

  /*! Returns the path to UTF-8 encoded string in generic format
   *
   * The encoding is always UTF-8.
   */
  std::string generic_u8string() const;

#if defined(FSPP_SUPPORT_WSTRING_API)
  std::wstring wstring() const;

  /*! Returns the path to wstring in generic format */
  std::wstring generic_wstring() const;
#endif

  /*! Compares the lexical representations of the path and another path.
   *
   * @returns 0 if *this and @p p are equal, -1 if @p p is less and 1 if greater than
   * *this.
   */
  int compare(const path& p) const;
  int compare(const string_type& str) const;
  int compare(const value_type* s) const;

  /*! Returns the root name of the path. If the path does not include root name, returns
   * path(). */
  path root_name() const;

  /*! Returns the root directory of the path. If the path does not include root name,
   * returns path(). */
  path root_directory() const;

  /*! Returns the root path of the path. If the path does not include root path, returns
   *  path().
   *
   * Equivalent to: <tt>root_name() / root_directory()</tt>
   */
  path root_path() const;

  /*! Returns path relative to root path. If @p *this is an empty path, returns an empty
   * path. */
  path relative_path() const;

  /*! Returns the path to the parent directory.
   *
   * Returns empty path if empty() or there's only a single element in the path.
   *
   * Equivalent to <tt>foreach (begin(), --end(), [&](auto p){ *this /= p; })(</tt>.
   */
  path parent_path() const;

  /*! Returns the filename component of the path.
   *
   * Equivalent to <tt>empty() ? path() : *--end().</tt>
   */
  path filename() const;

  /*! Returns the filename identified by the path stripped of its extension.
   *
   * Returns the substring from the beginning of filename() up to and not including the
   * last period (.) character.  If the filename is one of the special filesystem
   * components "." or "..", or if it has no periods, the function returns the entire
   * filename().
   *
   * @note this method is called stem() in ISO/IEC TS 18822:2015 and should be called
   *   basename().
   */
  path stem() const;

  /*! Returns the extension of the filename component of the path *this.
   *
   * If the filename() component of the path contains a period (.), and is not one of the
   * special filesystem elements "." or "..", then the extension is the substring
   * beginning at the rightmost period (including the period) and until the end of the
   * pathname.
   *
   * If the pathname is either "." or "..", or if filename() does not contain the "."
   * character, then empty path is returned.
   *
   * The extension as returned by this function includes a period to make it possible to
   * distinguish the file that ends with a period (function returns ".") from a file with
   * no extension (function returns "")
   */
  path extension() const;

  /*! Returns *this converted to normal form (no dot (except possibly one at the end) or
   * dot-dot elements, and if the last element is a non-root directory separator, dot is
   * added) */
  path lexically_normal() const;

  /*! Returns *this made relative to base. */
  path lexically_relative(const path& base) const;

  /*! If the value of lexically_relative(base) is not an empty path, return it. Otherwise
   * return *this. */
  path lexically_proximate(const path& base) const;

  /*! Checks whether root_path() is empty. */
  bool has_root_path() const;
  /*! Checks whether root_name() is empty. */
  bool has_root_name() const;
  /*! Checks whether root_directory() is empty. */
  bool has_root_directory() const;
  /*! Checks whether relative_path() is empty. */
  bool has_relative_path() const;
  /*! Checks whether parent_path() is empty. */
  bool has_parent_path() const;
  /*! Checks whether filename() is empty. */
  bool has_filename() const;
  /*! Checks whether stem() is empty. */
  bool has_stem() const;
  /*! Checks whether extension() is empty. */
  bool has_extension() const;

  /*! Indicates whether the path is absolute.
   *
   * An absolute path is such that the elements of root_path() uniquely identify a file
   * system location.
   */
  bool is_absolute() const;

  /*! Indicates whether the path is relative.
   *
   * Equivalent to <tt>!is_absolute()</tt>
   */
  bool is_relative() const;

  /*! Returns an iterator to the first element of the path. If the path is empty, the
   * returned iterator is equal to end().
   *
   * The sequence denoted by this pair of iterators consists of the following:
   *
   * 1) root-name (if any),
   * 2) root-directory (if any),
   * 3) sequence of file-names, omitting * any directory separators,
   * 4) If there is a directory separator after the last file-name in the
   *    path, the last element before the end iterator is a fictitious dot
   *    file name.
   */
  iterator begin() const;

  /*! Returns an iterator one past the last element of the path. Dereferencing this
   * iterator is undefined behavior. */
  iterator end() const;

  /*! Compares two paths lexicographically.
   *
   * Path equality and equivalence have different semantics.  Two path "a" and "b" are
   * never equal, but may be equivalent (in the light of symlinks).
   */
  friend bool operator==(const path& lhs, const path& rhs);
  /*! Compares two paths lexicographically. */
  friend bool operator!=(const path& lhs, const path& rhs);
  /*! Compares two paths lexicographically. */
  friend bool operator<(const path& lhs, const path& rhs);
  /*! Compares two paths lexicographically. */
  friend bool operator<=(const path& lhs, const path& rhs);
  /*! Compares two paths lexicographically. */
  friend bool operator>(const path& lhs, const path& rhs);
  /*! Compares two paths lexicographically. */
  friend bool operator>=(const path& lhs, const path& rhs);

  /*! Concatenates two path.
   *
   * Equivalent to <tt>path(lhs) /= rhs</tt>.
   */
  friend path operator/(const path& lhs, const path& rhs);
};


/*! Exchanges the state of @p lhs with that of @p rhs. */
void
swap(path& lhs, path& rhs);


/*! Performs stream output on the path @p p.
 *
 * The path itself is written to @p os, probably even quoted to protect from
 * whitespace.
 */
std::ostream&
operator<<(std::ostream& os, const path& p);

/*! Performs stream input on the path @p p.
 *
 * The path itself is read from @p is, probably even quoted to protect from whitespace.
 */
std::istream&
operator>>(std::istream& is, path& p);

#if defined(FSPP_SUPPORT_WSTRING_API)
std::wostream&
operator<<(std::wostream& os, const path& p);
std::wistream&
operator>>(std::wistream& os, const path& p);
#endif

/*! Constructs a path from a UTF-8 encoded string
 *
 * @p source can either be a std::string or a null-terminated 8-bit C string with utf-8
 * encoding.
 */
template <class Source>
path
u8path(const Source& source);

/*! Constructs a path p from a UTF-8 encoded sequence of chars, supplied as a [first,
 *  last) iterator pair.
 *
 * InputIt must meet the requirements of InputIterator.
 */
template <class InputIt>
path
u8path(InputIt first, InputIt last);


//----------------------------------------------------------------------------------------

class FSPP_API path::iterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = path;
  using difference_type = std::ptrdiff_t;
  using pointer = const path*;
  using reference = const path&;

  iterator(const iterator&) = default;
#if defined(FSPP_IS_VS2013)
  iterator(iterator&& rhs);
#else
  iterator(iterator&& rhs) = default;
#endif

  iterator& operator=(const iterator& rhs) = default;
#if defined(FSPP_IS_VS2013)
  iterator& operator=(iterator&& rhs);
#else
  iterator& operator=(iterator&& rhs) = default;
#endif
  bool operator==(const iterator& rhs) const;
  bool operator!=(const iterator& rhs) const;
  iterator& operator--();
  iterator operator--(int);
  iterator& operator++();
  iterator operator++(int);
  reference operator*();
  pointer operator->();

private:
  using string_type = path::string_type;
  friend class path;

  //! creates an iterator to the first element of @p p
  iterator(const path& p, string_type::const_iterator it);

  /*! pointer to the path to iterate */
  const string_type* _data = nullptr;
  /*! points to the start of the current element in _data */
  string_type::const_iterator _it;
  /*! the current element as returned by operator*() and operator->() */
  path _elt;
};


}  // namespace filesystem
}  // namespace eyestep

#include "fspp/details/path.ipp"
