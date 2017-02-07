// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/operations.hpp"

#include "operations_impl.hpp"
#include "vfs_private.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/types.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/utils.hpp"

#include <chrono>


namespace eyestep {
namespace filesystem {

FSPP_API path
absolute(const path& p, const path& base, std::error_code& ec)
{
  const auto absolute_if_rel = [](const path& base1, std::error_code& ec1) -> path {
    if (base1.is_absolute()) {
      ec1.clear();
      return base1;
    }
    else {
      const auto cp = current_path(ec1);
      if (ec1) {
        return {};
      }

      return absolute(base1, cp, ec1);
    }
  };

  const auto has_root_name = p.has_root_name();
  const auto has_root_dir = p.has_root_directory();

  if (has_root_name && has_root_dir) {
    ec.clear();
    return p;
  }
  else if (has_root_name && !has_root_dir) {
    const auto abs_base = absolute_if_rel(base, ec);
    return p.root_name() / abs_base.root_directory() / abs_base.relative_path()
           / p.relative_path();
  }
  else if (!has_root_name && has_root_dir) {
    return absolute_if_rel(base, ec).root_name() / p;
  }

  return absolute_if_rel(base, ec) / p;
}


path
absolute(const path& p, const path& base)
{
  std::error_code ec;
  const auto rv = absolute(p, base, ec);
  if (ec) {
    throw filesystem_error("can't make path absolute ", p, base, ec);
  }

  return rv;
}


void
copy(const path& from, const path& to)
{
  std::error_code ec;
  copy(from, to, copy_options::none, ec);
  if (ec) {
    throw filesystem_error("can't copy ", from, to, ec);
  }
}


void
copy(const path& from, const path& to, std::error_code& ec) NOEXCEPT
{
  copy(from, to, copy_options::none, ec);
}


void
copy(const path& from, const path& to, copy_options options)
{
  std::error_code ec;
  copy(from, to, options, ec);
  if (ec) {
    throw filesystem_error("can't copy ", from, to, ec);
  }
}


void
copy(const path& from, const path& to, copy_options options, std::error_code& ec) NOEXCEPT
{
  const auto copy_impl = [](file_status from_st, const path& from_p, const path& to_p,
                            copy_options opts, std::error_code& ec2) -> file_status {
    if (!exists(from_st)) {
      ec2 = std::make_error_code(std::errc::no_such_file_or_directory);
      return file_status{};
    }

    const auto to_st = (opts & copy_options::skip_symlinks) != 0
                           || (opts & copy_options::create_symlinks) != 0
                         ? symlink_status(to_p, ec2)
                         : status(to_p, ec2);
    if (ec2) {
      return file_status{};
    }

    if (is_other(from_st)) {
      ec2 = std::make_error_code(std::errc::operation_not_supported);
      return file_status{};
    }
    if (exists(to_st)) {
      if (equivalent(from_p, to_p, ec2)) {
        ec2 = std::make_error_code(std::errc::file_exists);
        return file_status{};
      }
      if (is_other(to_st)) {
        ec2 = std::make_error_code(std::errc::operation_not_supported);
        return file_status{};
      }

      if (is_directory(from_st) && is_regular_file(to_st)) {
        ec2 = std::make_error_code(std::errc::not_a_directory);
        return file_status{};
      }
    }

    if (is_symlink(from_st)) {
      if ((opts & copy_options::skip_symlinks) != 0) {
        // do nothing
        ec2.clear();
      }
      else if (!exists(to_st) && (opts & copy_options::copy_symlinks) != 0) {
        copy_symlink(from_p, to_p, ec2);
      }
      else {
        ec2 = std::make_error_code(std::errc::file_exists);
        return file_status{};
      }
    }
    else if (is_regular_file(from_st)) {
      if ((opts & copy_options::directories_only) != 0) {
        // do nothing
        ec2.clear();
      }
      else if ((opts & copy_options::create_symlinks) != 0) {
        if (!(from_p.is_absolute() || to_p.lexically_relative(from_p).empty())) {
          ec2 = std::make_error_code(std::errc::operation_not_supported);
          return file_status{};
        }
        create_symlink(from_p, to_p, ec2);
      }
      else if ((opts & copy_options::create_hard_links) != 0) {
        create_hard_link(from_p, to_p, ec2);
      }
      else if (is_directory(to_st)) {
        copy_file(from_p, to_p / from_p.filename(), opts, ec2);
      }
      else {
        copy_file(from_p, to_p, opts, ec2);
      }
    }
    else {
      ec2.clear();
    }

    return to_st;
  };

  const auto copy_dir_impl = [](const file_status& fs, const path& to_p,
                                const path& existing_p, std::error_code& ec2) {
    if (!exists(fs)) {
      create_directory(to_p, existing_p, ec2);
      if (ec2) {
        return;
      }
    }
  };

  const auto from_st = (options & copy_options::skip_symlinks) != 0
                           || (options & copy_options::create_symlinks) != 0
                         ? symlink_status(from, ec)
                         : status(from, ec);
  if (ec) {
    return;
  }

  const auto to_st = copy_impl(from_st, from, to, options, ec);
  if (ec) {
    return;
  }

  if (is_directory(from_st)
      && ((options & copy_options::recursive) != 0 || options == copy_options::none)) {
    copy_dir_impl(to_st, to, from, ec);
    if (ec) {
      return;
    }

    auto iter = recursive_directory_iterator(from, ec);
    if (ec) {
      return;
    }
    if (iter == end(iter)) {
      ec.clear();
      return;
    }

    auto last_depth = iter.depth();
    auto rel_path = path{};
    for (; iter != end(iter); ++iter) {
      auto& e = *iter;

      const auto& next_from = e.path();
      if (last_depth != iter.depth()) {
        rel_path = next_from.parent_path().lexically_relative(from);
        last_depth = iter.depth();
      }
      const auto next_to = to / rel_path / e.path().filename();

      const auto next_from_st = (options & copy_options::skip_symlinks) != 0
                                    || (options & copy_options::create_symlinks) != 0
                                  ? e.symlink_status(ec)
                                  : e.status(ec);
      const auto next_to_st = copy_impl(next_from_st, next_from, next_to, options, ec);
      if (ec) {
        return;
      }

      if (is_directory(next_from_st)) {
        if ((options & copy_options::recursive) != 0) {
          copy_dir_impl(next_to_st, next_to, next_from, ec);
          if (ec) {
            return;
          }
        }
        else {
          iter.disable_recursion_pending();
        }
      }
    }

    ec.clear();
  }
  else {
    // do nothing
    ec.clear();
  }
}


bool
copy_file(const path& from, const path& to)
{
  return copy_file(from, to, copy_options::none);
}


bool
copy_file(const path& from, const path& to, std::error_code& ec) NOEXCEPT
{
  return copy_file(from, to, copy_options::none, ec);
}


bool
copy_file(const path& from, const path& to, copy_options options)
{
  std::error_code ec;
  auto rv = copy_file(from, to, options, ec);
  if (ec) {
    throw filesystem_error("can't copy file", from, to, ec);
  }

  return rv;
}


bool
copy_file(const path& from,
          const path& to,
          copy_options options,
          std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<bool>(
        from, to, [&](vfs::IFilesystem& fs, const path& from2, const path& to2) {
          return fs.copy_file(from2, to2, options, ec);
        })) {
    return val.value();
  }

  return impl::copy_file(from, to, options, ec);
}


void
copy_symlink(const path& from, const path& to)
{
  std::error_code ec;
  copy_symlink(from, to, ec);
  if (ec) {
    throw filesystem_error("can't copy symlink", from, to, ec);
  }
}


void
copy_symlink(const path& from, const path& to, std::error_code& ec) NOEXCEPT
{
  auto froms = status(from, ec);
  if (ec) {
    return;
  }

  auto link_p = read_symlink(from, ec);
  if (ec) {
    return;
  }

  if (is_directory(froms)) {
    create_directory_symlink(link_p, to, ec);
  }
  else {
    create_symlink(link_p, to, ec);
  }
}


bool
create_directory(const path& p)
{
  std::error_code ec;
  auto rv = create_directory(p, ec);
  if (ec) {
    throw filesystem_error("can't create directory", p, ec);
  }

  return rv;
}


bool
create_directory(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<bool>(p, [&](vfs::IFilesystem& fs, const path& p2) {
        return fs.create_directory(p2, ec);
      })) {
    return val.value();
  }

  return impl::create_directory(p, ec);
}


bool
create_directory(const path& p, const path& existing_p)
{
  std::error_code ec;
  auto rv = create_directory(p, existing_p, ec);
  if (ec) {
    throw filesystem_error("can't create directory", p, existing_p, ec);
  }

  return rv;
}


bool
create_directory(const path& p, const path& existing_p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<bool>(
        p, existing_p,
        [&](vfs::IFilesystem& fs, const path& p2, const path& existing_p2) {
          return fs.create_directory(p2, existing_p2, ec);
        })) {
    return val.value();
  }

  return impl::create_directory(p, existing_p, ec);
}


bool
create_directories(const path& p)
{
  std::error_code ec;
  auto rv = create_directories(p, ec);
  if (ec) {
    throw filesystem_error("can't create directory", p, ec);
  }

  return rv;
}


bool
create_directories(const path& p, std::error_code& ec) NOEXCEPT
{
  auto fs = status(p, ec);
  if (ec) {
    return false;
  }

  if (is_directory(fs)) {
    return false;
  }

  auto pp = p.parent_path();
  if (!pp.empty()) {
    create_directories(pp, ec);
    if (ec) {
      return false;
    }

    create_directory(p, ec);
    if (ec) {
      return false;
    }

    return true;
  }

  ec = std::make_error_code(std::errc::not_a_directory);
  return false;
}


void
create_hard_link(const path& target_p, const path& link_p)
{
  std::error_code ec;
  create_hard_link(target_p, link_p, ec);
  if (ec) {
    throw filesystem_error("can't create link", target_p, link_p, ec);
  }
}


void
create_hard_link(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(
        target_p, link_p,
        [&](vfs::IFilesystem& fs, const path& target_p2, const path& link_p2) {
          fs.create_hard_link(target_p2, link_p2, ec);
          return false;
        })) {
    impl::create_hard_link(target_p, link_p, ec);
  }
}


void
create_symlink(const path& target_p, const path& link_p)
{
  std::error_code ec;
  create_symlink(target_p, link_p, ec);
  if (ec) {
    throw filesystem_error("can't create symlink", target_p, link_p, ec);
  }
}


void
create_symlink(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(
        target_p, link_p,
        [&](vfs::IFilesystem& fs, const path& target_p2, const path& link_p2) {
          fs.create_symlink(target_p2, link_p2, ec);
          return false;
        })) {
    impl::create_symlink(target_p, link_p, ec);
  }
}


void
create_directory_symlink(const path& target_p, const path& link_p)
{
  std::error_code ec;
  create_directory_symlink(target_p, link_p, ec);
  if (ec) {
    throw filesystem_error("can't create directory symlink", target_p, link_p, ec);
  }
}


void
create_directory_symlink(const path& target_p,
                         const path& link_p,
                         std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(
        target_p, link_p,
        [&](vfs::IFilesystem& fs, const path& target_p2, const path& link_p2) {
          fs.create_directory_symlink(target_p2, link_p2, ec);
          return false;
        })) {
    impl::create_directory_symlink(target_p, link_p, ec);
  }
}


path
current_path()
{
  std::error_code ec;
  auto rv = current_path(ec);
  if (ec) {
    throw filesystem_error("can't read the current path", ec);
  }

  return rv;
}


void
current_path(const path& p)
{
  std::error_code ec;
  current_path(p, ec);
  if (ec) {
    throw filesystem_error("can't set the current path", ec);
  }
}


bool
equivalent(const path& p1, const path& p2)
{
  std::error_code ec;
  auto rv = equivalent(p1, p2, ec);
  if (ec) {
    throw filesystem_error("can't determine whether paths are equivalent", p1, p2, ec);
  }

  return rv;
}


bool
equivalent(const path& p1, const path& p2, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<bool>(
        p1, p2, [&](vfs::IFilesystem& fs, const path& rp1, const path& rp2) {
          return fs.equivalent(rp1, rp2, ec);
        })) {
    return val.value();
  }

  return impl::equivalent(p1, p2, ec);
}


bool
exists(file_status s) NOEXCEPT
{
  return status_known(s) && s.type() != file_type::not_found;
}


bool
exists(const path& p)
{
  return exists(status(p));
}


bool
exists(const path& p, std::error_code& ec) NOEXCEPT
{
  return exists(status(p, ec));
}


file_size_type
file_size(const path& p)
{
  std::error_code ec;
  auto rv = file_size(p, ec);
  if (ec) {
    throw filesystem_error("can't determine file size", p, ec);
  }

  return rv;
}


file_size_type
file_size(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<file_size_type>(
        p, [&](vfs::IFilesystem& fs, const path& p2) { return fs.file_size(p2, ec); })) {
    return val.value();
  }

  return impl::file_size(p, ec);
}


std::uintmax_t
hard_link_count(const path& p)
{
  std::error_code ec;
  auto rv = hard_link_count(p, ec);
  if (ec) {
    throw filesystem_error("can't determine the hard link count", p, ec);
  }

  return rv;
}


std::uintmax_t
hard_link_count(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val =
        vfs::with_vfs_do<std::uintmax_t>(p, [&](vfs::IFilesystem& fs, const path& p2) {
          return fs.hard_link_count(p2, ec);
        })) {
    return val.value();
  }

  return impl::hard_link_count(p, ec);
}


file_time_type
last_write_time(const path& p)
{
  std::error_code ec;
  auto rv = last_write_time(p, ec);
  if (ec) {
    throw filesystem_error("can't determine last write time", p, ec);
  }

  return rv;
}


file_time_type
last_write_time(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val =
        vfs::with_vfs_do<file_time_type>(p, [&](vfs::IFilesystem& fs, const path& p2) {
          return fs.last_write_time(p2, ec);
        })) {
    return val.value();
  }

  return impl::last_write_time(p, ec);
}


void
last_write_time(const path& p, file_time_type new_time)
{
  std::error_code ec;
  last_write_time(p, new_time, ec);
  if (ec) {
    throw filesystem_error("can't set write time", p, ec);
  }
}


void
last_write_time(const path& p, file_time_type new_time, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(p, [&](vfs::IFilesystem& fs, const path& p2) {
        fs.last_write_time(p2, new_time, ec);
        return false;
      })) {
    impl::last_write_time(p, new_time, ec);
  }
}


void
permissions(const path& p, perms prms)
{
  std::error_code ec;
  permissions(p, prms, ec);
  if (ec) {
    throw filesystem_error("can't set write time", p, ec);
  }
}


void
permissions(const path& p, perms prms, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(p, [&](vfs::IFilesystem& fs, const path& p2) {
        fs.permissions(p2, prms, ec);
        return false;
      })) {
    impl::permissions(p, prms, ec);
  }
}


path
read_symlink(const path& p)
{
  std::error_code ec;
  auto rv = read_symlink(p, ec);
  if (ec) {
    throw filesystem_error("failed to read symlink", p, ec);
  }

  return rv;
}


path
read_symlink(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<path>(p, [&](vfs::IFilesystem& fs, const path& p2) {
        return fs.read_symlink(p2, ec);
      })) {
    return val.value();
  }

  return impl::read_symlink(p, ec);
}


bool
remove(const path& p)
{
  std::error_code ec;
  auto rv = remove(p, ec);
  if (ec) {
    throw filesystem_error("failed to remove path", p, ec);
  }

  return rv;
}


bool
remove(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<bool>(
        p, [&](vfs::IFilesystem& fs, const path& p2) { return fs.remove(p2, ec); })) {
    return val.value();
  }

  return impl::remove(p, ec);
}


std::uintmax_t
remove_all(const path& p)
{
  std::error_code ec;
  auto rv = remove_all(p, ec);
  if (ec) {
    throw filesystem_error("failed to remove path recursively", p, ec);
  }

  return rv;
}


std::uintmax_t
remove_all(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<std::uintmax_t>(
        p, [&](vfs::IFilesystem& fs, const path& p2) { return fs.remove_all(p2, ec); })) {
    return val.value();
  }

  return impl::remove_all(p, ec);
}


void
rename(const path& old_p, const path& new_p)
{
  std::error_code ec;
  rename(old_p, new_p, ec);
  if (ec) {
    throw filesystem_error("failed to rename", old_p, new_p, ec);
  }
}


void
rename(const path& old_p, const path& new_p, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(
        old_p, new_p, [&](vfs::IFilesystem& fs, const path& old_p2, const path& new_p2) {
          fs.rename(old_p2, new_p2, ec);
          return false;
        })) {
    impl::rename(old_p, new_p, ec);
  }
}


void
resize_file(const path& p, file_size_type new_size)
{
  std::error_code ec;
  resize_file(p, new_size, ec);
  if (ec) {
    throw filesystem_error("can't resize file", p, ec);
  }
}


void
resize_file(const path& p, file_size_type new_size, std::error_code& ec) NOEXCEPT
{
  if (!vfs::with_vfs_do<bool>(p, [&](vfs::IFilesystem& fs, const path& p2) {
        fs.resize_file(p2, new_size, ec);
        return false;
      })) {
    impl::resize_file(p, new_size, ec);
  }
}


space_info
space(const path& p)
{
  std::error_code ec;
  auto rv = space(p, ec);
  if (ec) {
    throw filesystem_error("can't determine space of filesystem", p, ec);
  }

  return rv;
}


space_info
space(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<space_info>(
        p, [&](vfs::IFilesystem& fs, const path& p2) { return fs.space(p2, ec); })) {
    return val.value();
  }

  return impl::space(p, ec);
}


file_status
status(const path& p)
{
  std::error_code ec;
  auto rv = status(p, ec);
  if (ec) {
    throw filesystem_error("can't read file status", p, ec);
  }

  return rv;
}


file_status
status(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val = vfs::with_vfs_do<file_status>(
        p, [&](vfs::IFilesystem& fs, const path& p2) { return fs.status(p2, ec); })) {
    return val.value();
  }

  return impl::status(p, ec);
}


file_status
symlink_status(const path& p)
{
  std::error_code ec;
  auto rv = symlink_status(p, ec);
  if (ec) {
    throw filesystem_error("can't read file status", p, ec);
  }

  return rv;
}


file_status
symlink_status(const path& p, std::error_code& ec) NOEXCEPT
{
  if (auto val =
        vfs::with_vfs_do<file_status>(p, [&](vfs::IFilesystem& fs, const path& p2) {
          return fs.symlink_status(p2, ec);
        })) {
    return val.value();
  }

  return impl::symlink_status(p, ec);
}


path
system_complete(const path& p)
{
  std::error_code ec;
  auto rv = system_complete(p, ec);
  if (ec) {
    throw filesystem_error("can't make complete path", p, ec);
  }

  return rv;
}


path
temp_directory_path()
{
  std::error_code ec;
  auto rv = temp_directory_path(ec);
  if (ec) {
    throw filesystem_error("failed to get temp path", ec);
  }

  return rv;
}


void
touch(const path& p)
{
  std::error_code ec;
  touch(p, ec);
  if (ec) {
    throw filesystem_error("failed to touch path", p, ec);
  }
}


void
touch(const path& p, std::error_code& ec) NOEXCEPT
{
  auto fs = status(p, ec);
  if (ec) {
    return;
  }
  if (fs.type() == file_type::not_found) {
    with_stream_for_writing(p, ec, [](std::ostream&) {});
  }
  else {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    last_write_time(p, now, ec);
  }
}


bool
status_known(file_status s) NOEXCEPT
{
  return s.type() != file_type::none;
}


bool
is_block_file(file_status s) NOEXCEPT
{
  return s.type() == file_type::block;
}


bool
is_block_file(const path& p)
{
  return is_block_file(status(p));
}


bool
is_block_file(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_block_file(status(p, ec));
}


bool
is_character_file(file_status s) NOEXCEPT
{
  return s.type() == file_type::character;
}


bool
is_character_file(const path& p)
{
  return is_character_file(status(p));
}


bool
is_character_file(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_character_file(status(p, ec));
}


bool
is_directory(file_status s) NOEXCEPT
{
  return s.type() == file_type::directory;
}


bool
is_directory(const path& p)
{
  return is_directory(status(p));
}


bool
is_directory(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_directory(status(p, ec));
}


bool
is_empty(const path& p)
{
  std::error_code ec;
  auto rv = is_empty(p, ec);
  if (ec) {
    throw filesystem_error("can't check emptiness", p, ec);
  }
  return rv;
}


bool
is_empty(const path& p, std::error_code& ec)
{
  auto is_dir = is_directory(status(p, ec));
  if (ec) {
    return false;
  }

  if (is_dir) {
    auto iter = directory_iterator(p, ec);
    if (ec) {
      return false;
    }

    return iter == end(iter);
  }

  auto fs = file_size(p, ec);
  if (ec) {
    return false;
  }

  ec.clear();
  return fs == 0;
}


bool
is_fifo(file_status s) NOEXCEPT
{
  return s.type() == file_type::fifo;
}


bool
is_fifo(const path& p)
{
  return is_fifo(status(p));
}


bool
is_fifo(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_fifo(status(p, ec));
}


bool
is_other(file_status s)
{
  return exists(s) && !is_regular_file(s) && !is_directory(s) && !is_symlink(s);
}


bool
is_other(const path& p)
{
  return is_other(status(p));
}


bool
is_other(const path& p, std::error_code& ec)
{
  return is_other(status(p, ec));
}


bool
is_regular_file(file_status s) NOEXCEPT
{
  return s.type() == file_type::regular;
}


bool
is_regular_file(const path& p)
{
  return is_regular_file(status(p));
}


bool
is_regular_file(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_regular_file(status(p, ec));
}


bool
is_socket(file_status s) NOEXCEPT
{
  return s.type() == file_type::socket;
}


bool
is_socket(const path& p)
{
  return is_socket(status(p));
}


bool
is_socket(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_socket(status(p, ec));
}


bool
is_symlink(file_status s) NOEXCEPT
{
  return s.type() == file_type::symlink;
}


bool
is_symlink(const path& p)
{
  return is_symlink(status(p));
}


bool
is_symlink(const path& p, std::error_code& ec) NOEXCEPT
{
  return is_symlink(status(p, ec));
}

}  // namespace filesystem
}  // namespace eyestep
