// Copyright (c) 2016 Gregor Klinke

#include "memory_vfs.hpp"

#include "common.hpp"
#include "dir_iterator_private.hpp"
#include "vfs_private.hpp"

#include "fspp/estd/memory.hpp"
//#include "fspp/details/dir_iterator.hpp"
#include "fspp/details/file.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"

#include <cassert>
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>


namespace eyestep {
namespace filesystem {
namespace vfs {

namespace {

std::shared_ptr<FSNode>
create_regular_file_node(const std::shared_ptr<FSNode>& parent, std::string name)
{
  assert(parent->_type == file_type::directory);
  assert(name != k_dot.string() && name != k_dotdot.string());
  parent->_children[name] = std::make_shared<FSNode>(file_type::regular);
  return parent->_children[name];
}


std::shared_ptr<FSNode>
create_directory_node(const std::shared_ptr<FSNode>& parent, std::string name)
{
  assert(parent->_type == file_type::directory);
  if (name != k_dot.string() && name != k_dotdot.string()) {
    parent->_children[name] = std::make_shared<FSNode>(file_type::directory);
    return parent->_children[name];
  }
  return parent;
}


void
remove_node(FSNode& parent, const std::string& name)
{
  assert(parent._type == file_type::directory);
  parent._children.erase(name);
}


void
move_node(FSNode& srcparent,
          const std::string& srcname,
          FSNode& dstparent,
          const std::string& dstname)
{
  if (!(&srcparent == &dstparent && srcname == dstname)) {
    assert(srcparent._children.find(srcname) != end(srcparent._children));
    assert(dstparent._children.find(dstname) == end(dstparent._children));

    dstparent._children[dstname] = srcparent._children[srcname];
    srcparent._children.erase(srcname);
  }
}


void
replace_node(FSNode& srcparent,
             const std::string& srcname,
             FSNode& dstparent,
             const std::string& dstname)
{
  if (!(&srcparent == &dstparent && srcname == dstname)) {
    assert(dstparent._children.find(dstname) != end(dstparent._children));

    remove_node(dstparent, dstname);
    move_node(srcparent, srcname, dstparent, dstname);
  }
}


std::shared_ptr<FSNode>
find_node(const std::shared_ptr<FSNode>& base,
          const path& path,
          std::error_code& ec,
          bool create_nodes = false)
{
  using std::begin;
  using std::end;

  std::vector<std::shared_ptr<FSNode>> stack;

  std::shared_ptr<FSNode> node = base;
  stack.push_back(node);

  auto i_path = begin(path);
  auto i_pathend = end(path);
  while (i_path != i_pathend) {
    if (node->_type == file_type::directory) {
      auto nd_name = i_path->string();
      if (nd_name == k_dot.string()) {
        ++i_path;
      }
      else if (nd_name == k_dotdot.string()) {
        if (!stack.empty()) {
          node = stack.back();
          stack.pop_back();
          ++i_path;
        }
        else {
          ec = std::make_error_code(std::errc::no_such_file_or_directory);
          return nullptr;
        }
      }
      else {
        auto i_sub = node->_children.find(nd_name);
        if (i_sub != node->_children.end()) {
          stack.push_back(node);
          node = i_sub->second;
          ++i_path;
        }
        else {
          if (create_nodes) {
            stack.push_back(node);
            node = create_directory_node(node, nd_name);
            ++i_path;
          }
          else {
            ec = std::make_error_code(std::errc::no_such_file_or_directory);
            return nullptr;
          }
        }
      }
    }
    else {
      ec = std::make_error_code(std::errc::not_a_directory);
      return nullptr;
    }
  }

  ec.clear();
  return node;
}


std::uintmax_t
count_descendants(const FSNode& node)
{
  std::uintmax_t result = 0;
  if (node._type == file_type::directory) {
    result = node._children.size();
    for (const auto& childp : node._children) {
      result += count_descendants(*childp.second);
    }
  }

  return result;
}


void
dump_node(const FSNode& node, std::ostream& os, size_t level)
{
  auto indent = std::string(level * 2, ' ');

  if (node._type == file_type::directory) {
    os << "/\n";

    for (const auto& f : node._children) {
      os << indent << f.first;
      dump_node(*f.second, os, level + 1);
    }
  }
  else {
    os << " [" << node._last_write_time << ", " << node._file_size << "by"
       << "]\n";
  }
}


class MemoryVfsDirIter : public directory_iterator::IDirIterImpl
{
public:
  MemoryVfsDirIter(const path& p, FSNode* parent)
    : _iter(parent->_children.begin())
    , _parent(parent)
    , _parent_path(p)
  {
  }

  void increment(std::error_code& ec) override
  {
    if (!is_end()) {
      ++_iter;
      ec.clear();
      _is_store_set = false;
    }
  }

  const directory_entry& object() const override
  {
    if (!_is_store_set) {
      _is_store_set = true;
      _store.assign(_parent_path / _iter->first, _iter->second->_file_size);
    }
    return _store;
  }

  bool is_end() const override { return _iter == _parent->_children.end(); }

  bool equal(const IDirIterImpl* other) const override
  {
    const auto otherMfs = dynamic_cast<const MemoryVfsDirIter*>(other);
    if (otherMfs) {
      return _iter == otherMfs->_iter;
    }

    return false;
  }

private:
  mutable directory_entry _store;
  mutable bool _is_store_set = false;
  std::unordered_map<std::string, std::shared_ptr<FSNode>>::const_iterator _iter;
  FSNode* _parent;
  path _parent_path;
};

}  // namespace


//----------------------------------------------------------------------------------------

MemoryFilesystem::MemoryFilesystem()
  : _root(std::make_shared<FSNode>(file_type::directory))
{
  create_directory_node(_root, "/");
}


std::shared_ptr<FSNode>
MemoryFilesystem::root_node()
{
  return _root;
}


std::unique_ptr<detail::IFileImpl>
MemoryFilesystem::make_file_impl()
{
  return estd::make_unique<MemoryVfsFileImpl>(this);
}


std::unique_ptr<directory_iterator::IDirIterImpl>
MemoryFilesystem::make_dir_iterator(const path& p, std::error_code& ec)
{
  if (auto nd = find_node(root_node(), vfs::deroot(p), ec)) {
    if (nd->_type == file_type::directory) {
      ec.clear();

      // pass the non-derooted p in here
      return std::unique_ptr<directory_iterator::IDirIterImpl>(
        new MemoryVfsDirIter(p, nd.get()));
    }
    else {
      ec = std::make_error_code(std::errc::not_a_directory);
    }
  }

  return {};
}


void
MemoryFilesystem::dump(std::ostream& os)
{
  os << "-----------------------------------------------------------------\n";
  std::error_code ec;
  dump_node(*find_node(_root, "/", ec), os, 1);
  os << "-----------------------------------------------------------------\n";
}


path
MemoryFilesystem::canonical(const path& p, const path& base, std::error_code& ec)
{
  (void)p;
  (void)base;
  ec = std::make_error_code(std::errc::function_not_supported);
  return path();
}


bool
MemoryFilesystem::copy_file(const path& from,
                            const path& to,
                            copy_options options,
                            std::error_code& ec)
{
  if (auto srcnode = find_node(_root, from, ec)) {
    if (srcnode->_type == file_type::regular) {
      auto ds = status(to, ec);
      if (ds.type() == file_type::not_found) {
        auto dstparent = find_node(_root, to.parent_path(), ec);
        auto newnode = create_regular_file_node(dstparent, to.filename().string());
        newnode->copy_from_other(*srcnode);
        ec.clear();
        return true;
      }
      else if (ds.type() == file_type::regular) {
        if ((options & copy_options::overwrite_existing) != 0) {
          auto dstnode = find_node(_root, to, ec);
          dstnode->copy_from_other(*srcnode);
          ec.clear();
          return true;
        }
        else if ((options & copy_options::skip_existing) != 0) {
          ec.clear();
          return false;
        }
        else if ((options & copy_options::update_existing) != 0) {
          ec.clear();
          auto dstnode = find_node(_root, to, ec);
          if (srcnode->_last_write_time > dstnode->_last_write_time) {
            dstnode->copy_from_other(*srcnode);
            return true;
          }
          else {
            return false;
          }
        }
        else  // if (options & copy_options::none)
        {
          ec = std::make_error_code(std::errc::file_exists);
        }
      }
      else {
        ec = std::make_error_code(std::errc::is_a_directory);
      }
    }
    else {
      ec = std::make_error_code(std::errc::is_a_directory);
    }
  }
  else {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
  }

  return false;
}


void
MemoryFilesystem::copy_symlink(const path& from, const path& to, std::error_code& ec)
{
  (void)from;
  (void)to;
  ec = std::make_error_code(std::errc::function_not_supported);
}


bool
MemoryFilesystem::create_directory(const path& p, std::error_code& ec)
{
  auto s = status(p, ec);
  if (s.type() == file_type::not_found) {
    if (auto parent = find_node(_root, p.parent_path(), ec)) {
      create_directory_node(parent, p.filename().string());
      ec.clear();
      return true;
    }
    else {
      ec = std::make_error_code(std::errc::not_a_directory);
    }
  }
  else if (s.type() == file_type::directory) {
    ec.clear();
  }
  else {
    ec = std::make_error_code(std::errc::file_exists);
  }

  return false;
}


bool
MemoryFilesystem::create_directory(const path& p,
                                   const path& existing_p,
                                   std::error_code& ec)
{
  // TODO: we don't support dir attributes in the memory vfs currently
  (void)existing_p;
  return create_directory(p, ec);
}


bool
MemoryFilesystem::create_directories(const path& path, std::error_code& ec)
{
  return find_node(_root, path, ec, true) != nullptr;
}


void
MemoryFilesystem::create_hard_link(const path& target,
                                   const path& link,
                                   std::error_code& ec)
{
  (void)target;
  (void)link;
  ec = std::make_error_code(std::errc::function_not_supported);
}


void
MemoryFilesystem::create_symlink(const path& target,
                                 const path& link,
                                 std::error_code& ec)
{
  (void)target;
  (void)link;
  ec = std::make_error_code(std::errc::function_not_supported);
}


void
MemoryFilesystem::create_directory_symlink(const path& target,
                                           const path& link,
                                           std::error_code& ec)
{
  (void)target;
  (void)link;
  ec = std::make_error_code(std::errc::function_not_supported);
}


bool
MemoryFilesystem::equivalent(const path& p1, const path& p2, std::error_code& ec)
{
  auto n1 = find_node(_root, p1, ec);
  if (n1 && !ec) {
    auto n2 = find_node(_root, p2, ec);
    if (n2 && !ec) {
      ec.clear();
      return (n1 == n2);
    }
  }

  ec = std::make_error_code(std::errc::no_such_file_or_directory);
  return false;
}


file_size_type
MemoryFilesystem::file_size(const path& p, std::error_code& ec)
{
  if (auto nd = find_node(_root, p, ec)) {
    ec.clear();
    return nd->_file_size;
  }
  return file_size_type();
}


std::uintmax_t
MemoryFilesystem::hard_link_count(const path& p, std::error_code& ec)
{
  if (auto nd = find_node(_root, p, ec)) {
    // TODO: we don't support hard-links yet.  Hardcode to 1 for now
    (void)nd;
    ec.clear();
    return 1;
  }
  return std::uintmax_t();
}


file_time_type
MemoryFilesystem::last_write_time(const path& p, std::error_code& ec)
{
  if (auto nd = find_node(_root, p, ec)) {
    ec.clear();
    return nd->_last_write_time;
  }

  return file_time_type();
}


void
MemoryFilesystem::last_write_time(const path& p,
                                  file_time_type new_time,
                                  std::error_code& ec)
{
  if (auto nd = find_node(_root, p, ec)) {
    nd->_last_write_time = new_time;
    ec.clear();
  }
}


void
MemoryFilesystem::permissions(const path& p, perms prms, std::error_code& ec)
{
  (void)p;
  (void)prms;
  ec = std::make_error_code(std::errc::function_not_supported);
}


path
MemoryFilesystem::read_symlink(const path& p, std::error_code& ec)
{
  (void)p;
  ec = std::make_error_code(std::errc::function_not_supported);
  return path();
}


bool
MemoryFilesystem::remove(const path& p, std::error_code& ec)
{
  auto s = status(p, ec);
  if (s.type() == file_type::regular) {
    if (auto parent = find_node(_root, p.parent_path(), ec)) {
      remove_node(*parent, p.filename().string());
      ec.clear();
      return true;
    }
  }
  else if (s.type() == file_type::directory) {
    auto parent = find_node(_root, p.parent_path(), ec);
    auto dir = find_node(parent, p.filename(), ec);
    if (dir->_children.empty()) {
      remove_node(*parent, p.filename().string());
      ec.clear();
      return true;
    }
    else {
      ec = std::make_error_code(std::errc::directory_not_empty);
    }
  }
  else {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
  }
  return false;
}


std::uintmax_t
MemoryFilesystem::remove_all(const path& p, std::error_code& ec)
{
  auto s = status(p, ec);
  if (s.type() == file_type::regular) {
    if (auto parent = find_node(_root, p.parent_path(), ec)) {
      remove_node(*parent, p.filename().string());
      ec.clear();
      return 1;
    }
  }
  else if (s.type() == file_type::directory) {
    auto parent = find_node(_root, p.parent_path(), ec);
    auto descs = count_descendants(*find_node(parent, p.filename(), ec));
    remove_node(*parent, p.filename().string());
    ec.clear();
    return 1 + descs;
  }
  else if (s.type() == file_type::not_found) {
    ec.clear();
    return 0;
  }
  else {
    ec = std::make_error_code(std::errc::not_supported);
  }

  // In case of error the standard requires this:
  return static_cast<std::uintmax_t>(-1);
}


void
MemoryFilesystem::rename(const path& old_p, const path& new_p, std::error_code& ec)
{
  auto s = status(old_p, ec);
  if (s.type() == file_type::directory) {
    auto srcparent = find_node(_root, old_p.parent_path(), ec);

    auto ds = status(new_p, ec);
    if (ds.type() == file_type::directory) {
      // POSIX allows to replace an empty directory, but Windows doesn't.  Maybe we
      // should fail here?
      ec = std::make_error_code(std::errc::directory_not_empty);
    }
    else if (ds.type() == file_type::not_found) {
      if (auto dstparent = find_node(_root, new_p.parent_path(), ec)) {
        if (dstparent->_type == file_type::directory) {
          move_node(
            *srcparent, old_p.filename().string(), *dstparent, new_p.filename().string());
          ec.clear();
        }
        else {
          ec = std::make_error_code(std::errc::not_a_directory);
        }
      }
      else {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
      }
    }
    else if (ds.type() == file_type::regular) {
      ec = std::make_error_code(std::errc::not_a_directory);
    }
    else {
      // a better error code?
      ec = std::make_error_code(std::errc::not_supported);
    }
  }
  else if (s.type() == file_type::regular) {
    auto srcparent = find_node(_root, old_p.parent_path(), ec);
    auto dstparent = find_node(_root, new_p.parent_path(), ec);

    auto ds = status(new_p, ec);
    if (ds.type() == file_type::regular) {
      if (dstparent) {
        // replace dst with src
        replace_node(
          *srcparent, old_p.filename().string(), *dstparent, new_p.filename().string());
        ec.clear();
      }
      else {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
      }
    }
    else if (ds.type() == file_type::not_found) {
      if (dstparent) {
        if (dstparent->_type == file_type::directory) {
          // move src to dst
          move_node(
            *srcparent, old_p.filename().string(), *dstparent, new_p.filename().string());
          ec.clear();
        }
        else {
          ec = std::make_error_code(std::errc::not_a_directory);
        }
      }
      else if (ds.type() == file_type::directory) {
        ec = std::make_error_code(std::errc::is_a_directory);
      }
      else {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
      }
    }
    else {
      // a better error code?
      ec = std::make_error_code(std::errc::not_supported);
    }
  }
  else if (s.type() == file_type::not_found) {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
  }
  else {
    ec = std::make_error_code(std::errc::not_supported);
  }
}


void
MemoryFilesystem::resize_file(const path& p, file_size_type new_size, std::error_code& ec)
{
  auto nd = find_node(_root, p, ec);
  if (nd) {
    if (nd->_type == file_type::regular) {
      ec.clear();
      if (new_size != nd->_file_size) {
        nd->_file_size = new_size;
        nd->_data.resize(static_cast<size_t>(new_size), '\0');
        nd->touch();
      }
    }
    else {
      ec = std::make_error_code(std::errc::not_supported);
    }
  }
  else {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
  }
}


space_info
MemoryFilesystem::space(const path& p, std::error_code& ec) NOEXCEPT
{
  (void)p;
  ec = std::make_error_code(std::errc::function_not_supported);
  return space_info();
}


file_status
MemoryFilesystem::status(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto nd = find_node(_root, p, ec)) {
    return file_status(nd->_type);
  }

  ec.clear();
  return file_status(file_type::not_found);
}


file_status
MemoryFilesystem::symlink_status(const path& p, std::error_code& ec) NOEXCEPT
{
  // we don't support symlinks (yet), so this is equivalent to status()
  return status(p, ec);
}


//----------------------------------------------------------------------------------------

std::unique_ptr<vfs::IFilesystem>
make_memory_filesystem()
{
  return estd::make_unique<MemoryFilesystem>();
}


//----------------------------------------------------------------------------------------

MemoryVfsFileImpl::MemoryVfsFileImpl(MemoryFilesystem* fs)
  : _fs(fs)
  , _stream(std::ios::in)
{
}


MemoryVfsFileImpl::~MemoryVfsFileImpl()
{
  if (is_open()) {
    std::error_code ec;
    close(ec);
  }
}


std::iostream&
MemoryVfsFileImpl::open(const path& vpath, std::ios::openmode mode, std::error_code& ec)
{
  assert(!is_open());

  const auto p = vfs::deroot(vpath);

  const bool is_read = (mode & std::ios::in) != 0;
  const bool is_write = (mode & (std::ios::out | std::ios::app)) != 0;

  if (is_read && !is_write) {
    auto nd = find_node(_fs->root_node(), p, ec);
    if (!ec) {
      if (nd->_type == file_type::regular) {
#if defined(FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN)
        _stream = std::stringstream(nd->_data, mode);
#else
        _stream.clear();
        _stream.str(nd->_data);
#endif
        ec.clear();
        _node = nd;
        _filemode = k_read;
      }
      else {
        ec = std::make_error_code(std::errc::is_a_directory);
      }
    }
    else {
      ec = std::make_error_code(std::errc::no_such_file_or_directory);
    }
  }
  else if (is_write) {
    auto nd = find_node(_fs->root_node(), p, ec);
    if (!ec) {
      if (nd->_type == file_type::regular) {
        ec.clear();
#if defined(FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN)
        _stream = std::stringstream(mode & std::ios::trunc ? "" : nd->_data, mode);
#else
        _stream.str(mode & std::ios::trunc ? "" : nd->_data);
        _stream.clear();
#endif
        _node = nd;
        _filemode = is_read ? k_readwrite : k_write;
      }
      else {
        ec = std::make_error_code(std::errc::is_a_directory);
      }
    }
    else {
      auto dstparent = find_node(_fs->root_node(), p.parent_path(), ec);
      if (!ec) {
        if (dstparent->_type == file_type::directory) {
          ec.clear();
          _node = create_regular_file_node(dstparent, p.filename().string());
#if defined(FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN)
          _stream = std::stringstream(mode);
#else
          _stream.str("");
          _stream.clear();
#endif
          _filemode = is_read ? k_readwrite : k_write;
        }
        else {
          ec = std::make_error_code(std::errc::not_a_directory);
        }
      }
      else {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
      }
    }
  }
  else {
    ec = std::make_error_code(std::errc::function_not_supported);
  }

  return _stream;
}


bool
MemoryVfsFileImpl::is_open() const
{
  return _filemode != k_not_open;
}


void
MemoryVfsFileImpl::close(std::error_code& ec)
{
  switch (_filemode) {
  case k_not_open:
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    break;
  case k_read:
    ec.clear();
    break;
  case k_write:
  case k_readwrite:
    assert(_node);
    _node->set_data(_stream.str());
    ec.clear();
    break;
  }

// There's no way to close a stringstream.  And std::ios::openmode doesn't seem to have
// an "invalid" state.  0 is actually "std::ios::app" in libc++, so that doesn't work.
// The best we can try here is to "open" the stringstream in "input" format.  So you
// could read from it (which always eof()s, because it's empty), but you can't write to
// it.
#if defined(FSPP_HAVE_STD_STRINGSTREAM_COPYASSIGN)
  _stream = std::stringstream(std::ios::in);
#else
  _stream.str("");
  _stream.clear();
#endif
  _node = nullptr;
  _filemode = k_not_open;
}


//----------------------------------------------------------------------------------------

path
canonicalize_memory_vfs_path(const path& p, std::error_code& ec)
{
  using std::begin;
  using std::end;

  path result;

  for (const auto& elt : p) {
    if (elt.native() == k_dot.native()) {
      // nop.  Leave this out
    }
    else if (elt.native() == k_dotdot.native()) {
      auto len = std::distance(begin(result), end(result));
      if (len == 2 && result.has_root_name() && result.has_root_directory()) {
        ec.clear();
        return result.root_path();
      }
      else if (len > 0) {
        result = result.parent_path();
      }
      else {
        ec = std::make_error_code(std::errc::filename_too_long);
        return path();
      }
    }
    else {
      result /= elt;
    }
  }

  ec.clear();
  return result;
}

}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep
