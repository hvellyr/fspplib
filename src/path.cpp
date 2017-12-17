// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/path.hpp"

#include "common.hpp"

#include "fspp/estd/algorithm.hpp"

#include <algorithm>
#include <iostream>


namespace eyestep {
namespace filesystem {

namespace {
using data_iterator = path::string_type::const_iterator;


inline bool
is_separator(path::value_type c)
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  return c == L'/' || c == path::preferred_separator;
#else
  return c == '/';
#endif
}


inline int
root_separator_length()
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  return 2;
#else
  return 1;
#endif
}


inline bool
is_drive_spec(data_iterator i_first, data_iterator i_end)
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  return std::distance(i_first, i_end) >= 2 && *std::next(i_first) == ':'
         && std::isalpha(*i_first);
#else
  (void)i_first;
  (void)i_end;
  return false;
#endif
}


inline bool
is_root_separator(data_iterator it, data_iterator i_first, data_iterator end)
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  return std::distance(i_first, it) == 2 && it != end && is_separator(*it)
         && is_drive_spec(i_first, it);
#else
  return it == i_first && it != end && is_separator(*it);
#endif
}


inline bool
is_net_separator(data_iterator it, data_iterator end)
{
  return (std::distance(it, end) > 2 && is_separator(*it) && is_separator(*std::next(it))
          && !is_separator(*std::next(it, 2)));
}


inline data_iterator
skip_separators_fwd(data_iterator it, data_iterator end)
{
  while (it != end && is_separator(*it)) {
    ++it;
  }

  return it;
}


inline data_iterator
skip_separators_bwd(data_iterator it, data_iterator i_begin)
{
  while (it != i_begin && is_separator(*it)) {
    --it;
  }
  return it;
}


inline auto
find_next(data_iterator i_first, data_iterator i_end) -> data_iterator
{
  i_first = skip_separators_fwd(i_first, i_end);

  return std::find_if(
    i_first, i_end, [&](path::value_type c) { return is_separator(c); });
}


inline auto
find_prev(data_iterator i_first, data_iterator i_begin) -> data_iterator
{
  while (i_first != i_begin && !is_separator(*i_first)) {
    --i_first;
  }
  return i_first;
}


inline auto
find_root_directory(data_iterator i_begin, data_iterator i_end) -> data_iterator
{
  if (is_net_separator(i_begin, i_end)) {
    auto i_net = skip_separators_fwd(i_begin, i_end);
    return find_next(i_net, i_end);
  }
  else if (is_drive_spec(i_begin, i_end)) {
    auto sep_len = root_separator_length();
    return std::next(i_begin, sep_len);
  }
  else if (i_begin != i_end && is_separator(*i_begin)) {
    return i_begin;
  }

  return i_end;
}


}  // anon namespace


path&
path::operator/=(const path& p)
{
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  const auto is_drive_spec = [](const string_type& d) {
    return d.size() == 2 && std::isalpha(d[0]) && d[1] == L':';
  };
#endif

  if (!p._data.empty() && !_data.empty() && _data.back() != preferred_separator
      && p._data.front() != preferred_separator
#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
      && !is_drive_spec(_data)
#endif
        ) {
    _data.append(1, preferred_separator);
  }

  _data.append(p._data);

  return *this;
}


int
path::compare(const path& p) const
{
  using std::begin;
  using std::end;

  auto i_first1 = begin(*this);
  auto i_last1 = end(*this);

  auto i_first2 = begin(p);
  auto i_last2 = end(p);

  while (i_first1 != i_last1 && i_first2 != i_last2) {
    if (i_first1->native() < i_first2->native()) {
      return -1;
    }
    if (i_first1->native() > i_first2->native()) {
      return 1;
    }

    ++i_first1;
    ++i_first2;
  }

  if (i_first1 == i_last1 && i_first2 == i_last2) {
    return 0;
  }

  return i_first1 == i_last1 ? -1 : 1;
}


bool
path::has_root_path() const
{
  return has_root_directory() || has_root_name();
}


path
path::root_path() const
{
  auto tmp = root_name();
  if (has_root_directory()) {
    tmp += root_directory();
  }
  return tmp;
}


bool
path::has_root_name() const
{
  using std::begin;
  using std::end;

  return is_net_separator(begin(_data), end(_data))
         || is_drive_spec(begin(_data), end(_data));
}


path
path::root_name() const
{
  if (has_root_name()) {
    return *begin();
  }

  return {};
}


bool
path::has_root_directory() const
{
  using std::begin;
  using std::end;

  auto i_root = find_root_directory(begin(_data), end(_data));
  return i_root != end(_data) ? is_separator(*i_root) : false;
}


path
path::root_directory() const
{
  using std::begin;
  using std::end;

  auto i_root = find_root_directory(begin(_data), end(_data));
  if (i_root != end(_data) && is_separator(*i_root)) {
    return path(i_root, std::next(i_root));
  }

  return {};
}


bool
path::has_relative_path() const
{
  using std::begin;
  using std::end;

  auto i_begin = begin(_data);
  auto i_end = end(_data);
  auto i_root = find_root_directory(i_begin, i_end);

  return (i_root != i_end && std::next(i_root) != i_end)
         || (i_begin != i_end && !is_separator(*i_begin)
             && !is_drive_spec(i_begin, i_end));
}


path
path::relative_path() const
{
  using std::begin;
  using std::end;

  auto i_begin = begin(_data);
  auto i_end = end(_data);
  auto i_root = find_root_directory(i_begin, i_end);
  if (i_root != i_end) {
    auto i_rel = skip_separators_fwd(i_root, i_end);
    return path(i_rel, i_end);
  }
  else if (i_begin != i_end && !is_separator(*i_begin)
           && !is_drive_spec(i_begin, i_end)) {
    return *this;
  }

  return {};
}


bool
path::has_parent_path() const
{
  return !parent_path().empty();
}


path
path::parent_path() const
{
  using std::begin;
  using std::end;
  using std::prev;

  auto i_begin = begin(_data);
  auto i_end = end(_data);

  if (i_begin == i_end) {
    return {};
  }

  auto it = (--end(*this))._it;
  if (it != i_begin && is_separator(*prev(it))) {
    auto i_last = skip_separators_bwd(prev(it), i_begin);
    auto i_root = find_prev(i_last, i_begin);

    if (i_root != i_begin && prev(i_root) == i_begin
        && is_net_separator(prev(i_root), i_end)) {
      return path(i_begin, it);
    }

    auto sep_len = root_separator_length();
    if (it != i_begin && std::distance(i_begin, prev(it)) == sep_len) {
      if (is_root_separator(prev(it), i_begin, i_end)) {
        return path(i_begin, it);
      }
    }

    return path(i_begin, next(i_last));
  }

  return path(i_begin, it);
}


bool
path::has_filename() const
{
  return !filename().empty();
}


path
path::filename() const
{
  return empty() ? path() : *--end();
}


bool
path::has_stem() const
{
  return !stem().empty();
}


path
path::stem() const
{
  auto nm = filename();
  if (nm == k_dot || nm == k_dotdot) {
    return nm;
  }

  auto n = nm._data.rfind(path::value_type('.'));
  if (n == std::string::npos) {
    return nm;
  }

  return path(nm._data.substr(0, n));
}


bool
path::has_extension() const
{
  return !extension().empty();
}


path
path::extension() const
{
  auto nm = filename();
  if (nm == k_dot || nm == k_dotdot) {
    return {};
  }

  auto n = nm._data.rfind(path::value_type('.'));
  if (n == std::string::npos) {
    return {};
  }

  return path(nm._data.substr(n));
}


path&
path::replace_extension(const path& replacement)
{
  auto tmp = has_extension() ? parent_path() / stem() : *this;
  if (!replacement.empty() && replacement.string().front() != path::value_type('.')) {
    tmp += path::value_type('.');
  }
  tmp += replacement;

  swap(tmp);
  return *this;
}


path
path::lexically_normal() const
{
  using std::begin;
  using std::end;
  using std::next;

  path result;

  auto it = begin(*this);
  auto i_end = end(*this);
  for (; it != i_end; ++it) {
    if (it->native() == k_dot.native() && std::next(it) != i_end) {
      // nop.  Leave this out
    }
    else if (it->native() == k_dotdot.native()) {
      auto len = std::distance(begin(result), end(result));
      if (len == 2 && result.has_root_name() && result.has_root_directory()) {
        // ???
        return result.root_path();
      }
      else if (len > 0) {
        result = result.parent_path();
      }
      else {
        result /= *it;
      }
    }
    else {
      result /= *it;
    }
  }

  return result;
}


path
path::lexically_relative(const path& base) const
{
  using std::begin;
  using std::end;

  auto mismatched = estd::mismatch(begin(*this), end(*this), begin(base), end(base),
                                   [](const path& a, const path& b) { return a == b; });
  if (mismatched.first == begin(*this) || mismatched.second == begin(base)) {
    return {};
  }
  else if (mismatched.first == end(*this) && mismatched.second == end(base)) {
    return k_dot;
  }

  path result;
  for (auto it = mismatched.second; it != end(base); ++it) {
    result /= k_dotdot;
  }
  for (auto it = mismatched.first; it != end(*this); ++it) {
    result /= *it;
  }

  return result;
}


path
path::lexically_proximate(const path& base) const
{
  auto val = lexically_relative(base);
  return !val.empty() ? val : *this;
}


//----------------------------------------------------------------------------------------

path::iterator::iterator(const path& p, string_type::const_iterator it)
  : _data(&p._data)
  , _it(it)
{
  using std::begin;
  using std::end;

  auto i_begin = begin(*_data);
  auto i_end = end(*_data);

  if (_it == i_begin) {
    if (is_net_separator(_it, i_end)) {
      _elt = path(_it, find_next(_it, i_end));
    }
    else if (is_drive_spec(it, i_end)) {
      _elt = path(_it, std::next(_it, root_separator_length()));
    }
    else if (_it != i_end && is_separator(*_it)) {
      _elt = path(_it, std::next(_it));
    }
    else {
      _it = skip_separators_fwd(_it, i_end);
      _elt = path(_it, find_next(_it, i_end));
    }
  }
}


auto path::iterator::operator--() -> iterator&
{
  using std::begin;
  using std::end;
  using std::prev;
  using std::next;

  auto i_begin = begin(*_data);
  auto i_end = end(*_data);

  if (_it == i_begin) {
    _elt.clear();
    return *this;
  }

  if (is_separator(*prev(_it))) {
    // special case: see whether there's a //net part left of us.
    auto i_net = skip_separators_bwd(std::prev(_it), i_begin);
    i_net = find_prev(i_net, i_begin);
    if (i_net != i_begin && std::prev(i_net) == i_begin
        && is_net_separator(std::prev(i_net), i_end)) {
      --_it;
      _elt = path(_it, std::next(_it));
      return *this;
    }

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
    auto sep_len = root_separator_length();
    auto it2 = prev(_it);
    if (std::distance(i_begin, it2) == sep_len) {
      if (is_drive_spec(i_begin, i_end)) {
        --_it;
        _elt = path(_it, std::next(_it));
        return *this;
      }
    }
#endif

    if (prev(_it) == i_begin) {
      _it = i_begin;
      _elt = path(_it, std::next(_it));
      return *this;
    }

    if (_it == i_end) {
      --_it;
      _elt = k_dot;
      return *this;
    }
  }
  --_it;

  // skip separators
  _it = skip_separators_bwd(_it, i_begin);

  const auto i_last = _it != i_end ? next(_it) : _it;
  // find previous separator
  _it = find_prev(_it, i_begin);

  if (is_separator(*_it)) {
    if (_it != i_begin && prev(_it) == i_begin && is_net_separator(prev(_it), i_end)) {
      --_it;
      _elt = path(_it, i_last);
      return *this;
    }
    if (_it == i_begin && next(_it) == i_last) {
      _elt = path(_it, std::next(_it));
      return *this;
    }

    ++_it;
  }

#if defined(FSPP_IS_WIN) || defined(FSPP_EMULATE_WIN_PATH)
  auto sep_len = root_separator_length();
  if (_it == i_begin && std::distance(_it, i_last) > sep_len && is_drive_spec(_it, i_last)
      && !is_separator(*next(_it, sep_len))) {
    _it = next(_it, sep_len);
    _elt = path(_it, i_last);
    return *this;
  }
#endif

  _elt = path(_it, i_last);
  return *this;
}


auto path::iterator::operator++() -> iterator&
{
  using std::begin;
  using std::end;
  using std::prev;
  using std::next;

  auto i_begin = begin(*_data);
  auto i_end = end(*_data);

  if (_it == i_end) {
    return *this;
  }
  if (next(_it) == i_end && is_separator(*_it)) {
    _it = i_end;
    _elt.clear();
    return *this;
  }

  // special case.  A / after a //net part becomes the root_directory
  if (is_net_separator(_it, i_end)) {
    auto i_root = skip_separators_fwd(_it, i_end);
    if (i_root != i_end) {
      i_root = find_next(i_root, i_end);
      if (i_root != i_end) {
        _it = i_root;
        _elt = path(_it, std::next(_it));

        return *this;
      }
    }
  }

  auto i_next = next(_it, static_cast<string_type::difference_type>(_elt._data.size()));
  if (is_root_separator(i_next, i_begin, i_end)) {
    _it = i_next;
    _elt = path(_it, std::next(_it));
    return *this;
  }

  _it = skip_separators_fwd(i_next, i_end);

  if (_it == i_end && is_separator(*prev(_it))) {
    --_it;
    _elt = k_dot;
  }
  else {
    _elt = path(_it, find_next(_it, i_end));
  }
  return *this;
}


}  // namespace filesystem
}  // namespace eyestep
