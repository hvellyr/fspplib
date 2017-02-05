// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/operations.hpp"

#include "operations_impl.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/types.hpp"
#include "fspp/estd/memory.hpp"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include <array>
#include <system_error>


#if defined(FSPP_IS_MAC)
#define FSPP_MAX_PATH PATH_MAX
#elif defined(FSPP_IS_UNIX)
#define FSPP_MAX_PATH 2048
#endif

namespace eyestep {
namespace filesystem {
namespace impl {

namespace {
perms
map_posix_permissions(mode_t mode)
{
  return static_cast<perms>(mode & 07777);
}


file_type
map_buf_mode(mode_t mode)
{
  if (S_ISREG(mode)) {
    return file_type::regular;
  }
  else if (S_ISDIR(mode)) {
    return file_type::directory;
  }
  else if (S_ISLNK(mode)) {
    return file_type::symlink;
  }
  else if (S_ISCHR(mode)) {
    return file_type::character;
  }
  else if (S_ISBLK(mode)) {
    return file_type::block;
  }
  else if (S_ISFIFO(mode)) {
    return file_type::fifo;
  }
  else if (S_ISSOCK(mode)) {
    return file_type::socket;
  }

  return file_type::unknown;
}


bool
copy_file_and_content(const path& from,
                      const path& to,
                      bool is_exclusive,
                      std::error_code& ec) NOEXCEPT
{
  auto is = ::open(from.c_str(), O_RDONLY);
  if (is < 0) {
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  struct stat from_buf;
  if (::stat(from.c_str(), &from_buf)) {
    ::close(is);
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  auto os = ::open(to.c_str(), is_exclusive ? (O_CREAT | O_WRONLY | O_TRUNC | O_EXCL)
                                            : (O_CREAT | O_WRONLY | O_TRUNC),
                   from_buf.st_mode);
  if (os < 0) {
    ec = std::error_code(errno, std::generic_category());
    ::close(is);
    return false;
  }

  auto buf = estd::make_unique<std::array<char, 16384>>();
  ssize_t bytes_read;
  do {
    bytes_read = ::read(is, buf.get(), 4096);
    if (bytes_read > 0) {
      auto write_ofs = 0;
      do {
        auto bytes_written =
          ::write(os, buf.get() + write_ofs, size_t(bytes_read - write_ofs));
        write_ofs += bytes_written;
      } while (write_ofs < bytes_read);
    }
    else if (bytes_read < 0) {
      ec = std::error_code(errno, std::generic_category());
      ::close(is);
      ::close(os);
      return false;
    }
  } while (bytes_read != 0);

  ::close(is);
  if (::close(os)) {
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  ec.clear();
  return true;
}

}  // anon namespace


bool
copy_file(const path& from,
          const path& to,
          copy_options options,
          std::error_code& ec) NOEXCEPT
{
  auto fs = impl::status(to, ec);
  if (ec) {
    return false;
  }
  if (fs.type() == file_type::not_found) {
    return copy_file_and_content(from, to, true, ec);
  }

  if (impl::equivalent(from, to, ec)) {
    ec = std::error_code(EINVAL, std::generic_category());
    return false;
  }
  else if ((options & copy_options::skip_existing) != 0) {
    ec.clear();
    return false;
  }
  else if ((options & copy_options::overwrite_existing) != 0) {
    return copy_file_and_content(from, to, false, ec);
  }
  else if ((options & copy_options::update_existing) != 0) {
    auto from_time = impl::last_write_time(from, ec);
    if (ec) {
      return false;
    }
    auto to_time = impl::last_write_time(to, ec);
    if (ec) {
      return false;
    }

    if (from_time > to_time) {
      return copy_file_and_content(from, to, false, ec);
    }

    ec.clear();
    return false;
  }

  ec = std::error_code(EINVAL, std::generic_category());
  return false;
}


bool
create_directory(const path& p, std::error_code& ec) NOEXCEPT
{
  if (::mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
    if (errno == EEXIST || errno == EISDIR) {
      ec.clear();
    }
    else {
      ec = std::error_code(errno, std::generic_category());
    }
    return false;
  }

  ec.clear();
  return true;
}


bool
create_directory(const path& p, const path& existing_p, std::error_code& ec) NOEXCEPT
{
  struct stat attributes;

  if (::stat(existing_p.c_str(), &attributes)) {
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  if (::mkdir(p.c_str(), attributes.st_mode)) {
    if (errno == EEXIST || errno == EISDIR) {
      ec.clear();
      return false;
    }
    else {
      ec = std::error_code(errno, std::generic_category());
      return false;
    }
  }

  ec.clear();
  return true;
}


void
create_hard_link(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (::link(target_p.c_str(), link_p.c_str())) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


void
create_symlink(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (::symlink(target_p.c_str(), link_p.c_str())) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


void
create_directory_symlink(const path& target_p,
                         const path& link_p,
                         std::error_code& ec) NOEXCEPT
{
  impl::create_symlink(target_p, link_p, ec);
}


bool
equivalent(const path& p1, const path& p2, std::error_code& ec) NOEXCEPT
{
  struct stat buf1;
  struct stat buf2;
  auto rv1 = ::stat(p1.c_str(), &buf1);
  auto rv2 = ::stat(p2.c_str(), &buf2);

  if (rv1 || rv2) {
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  ec.clear();
  return buf1.st_dev == buf2.st_dev && buf1.st_ino == buf2.st_ino;
}


file_size_type
file_size(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::stat(p.c_str(), &buf) == 0) {
    return static_cast<file_size_type>(buf.st_size);
  }

  ec = std::error_code(errno, std::generic_category());
  return static_cast<file_size_type>(-1);
}


std::uintmax_t
hard_link_count(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::stat(p.c_str(), &buf) == 0) {
    return static_cast<std::uintmax_t>(buf.st_nlink);
  }

  ec = std::error_code(errno, std::generic_category());
  return static_cast<std::uintmax_t>(-1);
}


file_time_type
last_write_time(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::stat(p.c_str(), &buf) == 0) {
    return static_cast<file_time_type>(buf.st_mtime);
  }

  ec = std::error_code(errno, std::generic_category());
  return {};
}


void
last_write_time(const path& p, file_time_type new_time, std::error_code& ec) NOEXCEPT
{
  // since POSIX's utime() sets both atime and mtime, we first have to get the
  // atime from the file and reset it to the utimbuf.

  struct stat buf;
  if (::stat(p.c_str(), &buf)) {
    ec = std::error_code(errno, std::generic_category());
    return;
  }

  struct utimbuf tb;
  tb.actime = buf.st_atime;
  tb.modtime = new_time;

  if (::utime(p.c_str(), &tb)) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


path
read_symlink(const path& p, std::error_code& ec) NOEXCEPT
{
  char buf[FSPP_MAX_PATH];
  auto rv = ::readlink(p.c_str(), buf, FSPP_MAX_PATH - 1);
  if (rv < 0) {
    ec = std::error_code(errno, std::generic_category());
    return {};
  }

  ec.clear();
  return path(std::string(buf, std::string::size_type(rv)));
}


bool
remove(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::stat(p.c_str(), &buf)) {
    ec = std::error_code(errno, std::generic_category());
    return false;
  }

  if (S_ISDIR(buf.st_mode)) {
    if (::rmdir(p.c_str())) {
      ec = std::error_code(errno, std::generic_category());
      return false;
    }
  }
  else {
    if (::unlink(p.c_str())) {
      ec = std::error_code(errno, std::generic_category());
      return false;
    }
  }

  ec.clear();
  return true;
}


void
permissions(const path& p, perms prms, std::error_code& ec) NOEXCEPT
{
  const auto get_status = [](const path& q, perms prms2, std::error_code& ec2) {
    return (prms2 & perms::resolve_symlinks) != 0 ? impl::status(q, ec2)
                                                  : impl::symlink_status(q, ec2);
  };

  mode_t mode;
  if ((prms & perms::add_perms) != 0 && (prms & perms::remove_perms) != 0) {
    ec = std::error_code(EINVAL, std::generic_category());
    return;
  }
  else if ((prms & perms::add_perms) != 0) {
    ec.clear();
    mode = mode_t(get_status(p, prms, ec).permissions() | (prms & perms::mask));
    if (ec) {
      return;
    }
  }
  else if ((prms & perms::remove_perms) != 0) {
    ec.clear();
    mode = mode_t(get_status(p, prms, ec).permissions() & ~(prms & perms::mask));
    if (ec) {
      return;
    }
  }
  else {
    mode = mode_t(prms & perms::mask);
  }

  // mac doesn't support fchmodat(), so fall back to chmod().
  if (::chmod(p.c_str(), mode)) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


namespace {
void
remove_dir_rec(const path& p, std::uintmax_t& count, std::error_code& ec)
{
  DIR* dirp = ::opendir(p.c_str());
  if (dirp) {
    struct dirent* direntp = nullptr;

    do {
      struct dirent f;

      if (::readdir_r(dirp, &f, &direntp)) {
        ec = std::error_code(errno, std::generic_category());
        return;
      }

      if (direntp) {
        if (::strcmp(direntp->d_name, "..") == 0 || ::strcmp(direntp->d_name, ".") == 0) {
          continue;
        }

        const auto subpath = p / direntp->d_name;
        if (direntp->d_type == DT_DIR) {
          remove_dir_rec(subpath, count, ec);
          if (ec) {
            return;
          }
        }
        else {
          if (::unlink(subpath.c_str())) {
            ec = std::error_code(errno, std::generic_category());
            return;
          }
          ++count;
        }
      }
    } while (direntp);

    ::closedir(dirp);

    if (::rmdir(p.c_str())) {
      ec = std::error_code(errno, std::generic_category());
      return;
    }
    ++count;
  }
  else {
    ec = std::error_code(errno, std::generic_category());
    return;
  }
}

}  // anon namespace


std::uintmax_t
remove_all(const path& p, std::error_code& ec) NOEXCEPT
{
  std::uintmax_t count = 0;

  remove_dir_rec(p, count, ec);

  return ec ? static_cast<std::uintmax_t>(-1) : count;
}


void
rename(const path& old_p, const path& new_p, std::error_code& ec) NOEXCEPT
{
  if (::rename(old_p.c_str(), new_p.c_str())) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


void
resize_file(const path& p, file_size_type new_size, std::error_code& ec) NOEXCEPT
{
  if (::truncate(p.c_str(), off_t(new_size))) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}


space_info
space(const path& p, std::error_code& ec) NOEXCEPT
{
  struct statvfs vfs;
  space_info result;

  if (::statvfs(p.c_str(), &vfs)) {
    ec = std::error_code(errno, std::generic_category());

    result.capacity = static_cast<std::uintmax_t>(-1);
    result.free = static_cast<std::uintmax_t>(-1);
    result.available = static_cast<std::uintmax_t>(-1);

    return result;
  }

  ec.clear();

  result.capacity = static_cast<std::uintmax_t>(vfs.f_blocks) * vfs.f_frsize;
  result.free = static_cast<std::uintmax_t>(vfs.f_bfree) * vfs.f_frsize;
  result.available = static_cast<std::uintmax_t>(vfs.f_bavail) * vfs.f_frsize;

  return result;
}


file_status
status(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::stat(p.c_str(), &buf) == 0) {
    const auto permissions = map_posix_permissions(buf.st_mode);

    ec.clear();
    return file_status(map_buf_mode(buf.st_mode), permissions);
  }

  if (errno == ENOENT) {
    ec.clear();
    return file_status(file_type::not_found);
  }

  ec = std::error_code(errno, std::generic_category());
  return file_status(file_type::none);
}


file_status
symlink_status(const path& p, std::error_code& ec) NOEXCEPT
{
  struct stat buf;
  if (::lstat(p.c_str(), &buf) == 0) {
    const auto permissions = map_posix_permissions(buf.st_mode);

    ec.clear();
    return file_status(map_buf_mode(buf.st_mode), permissions);
  }

  if (errno == ENOENT) {
    return file_status(file_type::not_found);
  }

  ec = std::error_code(errno, std::generic_category());
  return file_status(file_type::none);
}

}  // namespace impl


path
current_path(std::error_code& ec) NOEXCEPT
{
  char buf[PATH_MAX];
  if (auto rv = ::getcwd(buf, PATH_MAX)) {
    ec.clear();
    return path(rv);
  }

  ec = std::error_code(errno, std::generic_category());
  return {};
}


void
current_path(const path& p, std::error_code& ec) NOEXCEPT
{
  if (::chdir(p.c_str())) {
    ec = std::error_code(errno, std::generic_category());
  }
  else {
    ec.clear();
  }
}

}  // namespace filesystem
}  // namespace eyestep
