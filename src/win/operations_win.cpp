// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/operations.hpp"

#include "operations_impl.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/details/filesystem_error.hpp"
#include "fspp/details/types.hpp"
#include "fspp/estd/memory.hpp"
#include "fspp/utility/scope.hpp"

#include <windows.h>

#include <array>
#include <limits>
#include <system_error>
#include <tuple>
#include <vector>


// This data type is actually defined in the ntifs.h header.  That's not
// available though in normal build environment.
typedef struct _REPARSE_DATA_BUFFER
{
  ULONG ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  union
  {
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG Flags;
      WCHAR PathBuffer[1];
    } SymbolicLinkReparseBuffer;
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      WCHAR PathBuffer[1];
    } MountPointReparseBuffer;
    struct
    {
      UCHAR DataBuffer[1];
    } GenericReparseBuffer;
  };
} REPARSE_DATA_BUFFER;


namespace eyestep {
namespace filesystem {
namespace impl {

namespace {

utility::Scope
make_handle_scope(HANDLE handle)
{
  return utility::make_scope([handle]() {
    if (handle != INVALID_HANDLE_VALUE) {
      ::CloseHandle(handle);
    }
  });
}


perms
guess_permissions(const path& p, DWORD attr)
{
  auto result = perms::owner_read | perms::group_read | perms::others_read;

  auto ext = p.extension().c_str();
  if (wcscmp(ext, L".com") == 0 || wcscmp(ext, L".exe") == 0 || wcscmp(ext, L".bat") == 0
      || wcscmp(ext, L".cmd") == 0) {
    result = result | perms::owner_exec | perms::group_exec | perms::others_exec;
  }
  if ((attr & FILE_ATTRIBUTE_READONLY) != FILE_ATTRIBUTE_READONLY) {
    result = result | perms::owner_write | perms::group_write | perms::others_write;
  }

  return result;
}


bool
read_reparse_point_data(char* buffer, size_t bufsize, const path& p, std::error_code& ec)
{
  auto handle = ::CreateFileW(
    p.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
    nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return false;
  }
  auto guard = make_handle_scope(handle);

  DWORD retlen = 0;
  if (!::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                         nullptr,  // in buffer
                         0,        // in buffer size
                         buffer, static_cast<DWORD>(bufsize), &retlen,
                         nullptr)) {  // overlapped structure
    ec = std::error_code(::GetLastError(), std::system_category());
    return false;
  }

  ec.clear();
  return true;
}


bool
is_symlink_reparse_point(const path& p, std::error_code& ec)
{
  // From MSDN: "Reparse point data, including the tag and optional GUID, cannot exceed 16
  // kilobytes."
  const int MAX_REPARSE_DATA_SIZE = 16 * 1024;
  auto buffer = std::vector<char>(sizeof(REPARSE_DATA_BUFFER) + MAX_REPARSE_DATA_SIZE);

  if (read_reparse_point_data(buffer.data(), buffer.size(), p, ec)) {
    const auto* data = reinterpret_cast<const REPARSE_DATA_BUFFER*>(buffer.data());
    return data->ReparseTag == IO_REPARSE_TAG_SYMLINK
           || data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT;
  }

  return false;
}


path
read_reparse_point(const path& p, std::error_code& ec)
{
  // From MSDN: "Reparse point data, including the tag and optional GUID, cannot exceed 16
  // kilobytes."
  const int MAX_REPARSE_DATA_SIZE = 16 * 1024;
  auto buffer = std::vector<char>(sizeof(REPARSE_DATA_BUFFER) + MAX_REPARSE_DATA_SIZE);

  if (read_reparse_point_data(buffer.data(), buffer.size(), p, ec)) {
    ec.clear();
    const auto* data = reinterpret_cast<const REPARSE_DATA_BUFFER*>(buffer.data());
    if (data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
      const auto* buf =
        static_cast<const wchar_t*>(data->SymbolicLinkReparseBuffer.PathBuffer);
      auto ofs = data->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t);
      auto len = data->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t);

      return path(buf + ofs, buf + ofs + len);
    }
    else if (data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
      const auto* buf =
        static_cast<const wchar_t*>(data->MountPointReparseBuffer.PathBuffer);
      auto ofs = data->MountPointReparseBuffer.PrintNameOffset / sizeof(wchar_t);
      auto len = data->MountPointReparseBuffer.PrintNameLength / sizeof(wchar_t);

      return path(buf + ofs, buf + ofs + len);
    }
  }

  return path();
}


bool
copy_file_impl(const path& from, const path& to, bool overwrite, std::error_code& ec)
{
  if (!::CopyFileW(from.c_str(), to.c_str(), overwrite)) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return false;
  }

  ec.clear();
  return true;
}


enum class file_type_impl
{
  none = file_type::none,
  symlink = file_type::symlink,
  symlink_directory,
  directory = file_type::directory,
  regular = file_type::regular
};

file_type_impl
symlink_file_type(const path& p, DWORD& attr, std::error_code& ec) NOEXCEPT
{
  attr = ::GetFileAttributesW(p.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return file_type_impl::none;
  }

  // GetFileAttributesW behave actually like lstat, i.e. if p refers to a
  // symlink it returns the data for the symlink itself; try follow it.
  if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) {
    if (is_symlink_reparse_point(p, ec)) {
      return (attr & FILE_ATTRIBUTE_DIRECTORY) ? file_type_impl::symlink_directory
                                               : file_type_impl::symlink;
    }
  }

  ec.clear();
  if ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
    return file_type_impl::directory;
  }

  return file_type_impl::regular;
}


path
get_path(const path& p, std::error_code& ec)
{
  int loop_cnt = 0;
  auto q = p;
  while (loop_cnt < 31) {
    auto attr = ::GetFileAttributesW(q.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
      ec = std::error_code(::GetLastError(), std::system_category());
      return {};
    }

    // GetFileAttributesW behave actually like lstat, i.e. if p refers to a
    // symlink it returns the data for the symlink itself; try follow it.
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) {
      auto res = read_reparse_point(q, ec);
      if (ec) {
        return {};
      }
      if (!res.empty()) {
        q = res;
        ++loop_cnt;
        continue;
      }
    }

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
      ec.clear();
      return q;
    }

    ec.clear();
    return q;
  }

  ec = std::make_error_code(std::errc::too_many_symbolic_link_levels);
  return {};
}


path
get_symlink_path(const path& p, std::error_code& ec)
{
  ec.clear();
  return p;
}

}  // anon namespace


bool
copy_file(const path& from,
          const path& to,
          copy_options options,
          std::error_code& ec) NOEXCEPT
{
  auto to_fs = impl::status(to, ec);
  if (ec) {
    return false;
  }
  if (to_fs.type() == file_type::not_found) {
    return copy_file_impl(from, to, true, ec);
  }

  if (impl::equivalent(from, to, ec)) {
    ec = std::make_error_code(std::errc::invalid_argument);
    return false;
  }
  else if ((options & copy_options::skip_existing) != 0) {
    ec.clear();
    return false;
  }
  else if ((options & copy_options::overwrite_existing) != 0) {
    return copy_file_impl(from, to, false, ec);
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
      return copy_file_impl(from, to, false, ec);
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
  if (!::CreateDirectoryW(p.c_str(), nullptr)) {
    auto er = ::GetLastError();
    if (er == ERROR_ALREADY_EXISTS) {
      ec.clear();
    }
    else {
      ec = std::error_code(er, std::system_category());
    }
    return false;
  }

  ec.clear();
  return true;
}


bool
create_directory(const path& p, const path& existing_p, std::error_code& ec) NOEXCEPT
{
  if (!::CreateDirectoryExW(existing_p.c_str(), p.c_str(), nullptr)) {
    auto er = ::GetLastError();
    if (er == ERROR_ALREADY_EXISTS) {
      ec.clear();
    }
    else {
      ec = std::error_code(er, std::system_category());
    }
    return false;
  }

  ec.clear();
  return true;
}


void
create_hard_link(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (!::CreateHardLinkW(link_p.c_str(), target_p.c_str(), nullptr)) {
    ec = std::error_code(::GetLastError(), std::system_category());
  }
  else {
    ec.clear();
  }
}


void
create_symlink(const path& target_p, const path& link_p, std::error_code& ec) NOEXCEPT
{
  if (!::CreateSymbolicLinkW(link_p.c_str(), target_p.c_str(), 0x0)) {
    ec = std::error_code(::GetLastError(), std::system_category());
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
  if (!::CreateSymbolicLinkW(
        link_p.c_str(), target_p.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY)) {
    ec = std::error_code(::GetLastError(), std::system_category());
  }
  else {
    ec.clear();
  }
}


bool
equivalent(const path& p1, const path& p2, std::error_code& ec) NOEXCEPT
{
  std::error_code ec1;
  std::error_code ec2;

  const auto fs1 = impl::status(p1, ec1);
  if (ec1) {
    return false;
  }
  const auto fs2 = impl::status(p2, ec2);
  if (ec2) {
    return false;
  }

  // If neither p1 nor p2 exists, or if both exist but neither is a file,
  // directory, or symlink (as determined by is_other), an error is reported.
  if (fs1.type() == file_type::not_found && fs2.type() == file_type::not_found) {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
    return false;
  }
  else if (fs1.type() == file_type::not_found || fs2.type() == file_type::not_found) {
    ec.clear();
    return false;
  }
  else {
    if (is_other(fs1) || is_other(fs2)) {
      ec = std::make_error_code(std::errc::operation_not_supported);
      return false;
    }
  }

  auto handle1 =
    ::CreateFileW(p1.c_str(), FILE_READ_ATTRIBUTES,
                  FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  auto guard1 = make_handle_scope(handle1);

  auto handle2 =
    ::CreateFileW(p2.c_str(), FILE_READ_ATTRIBUTES,
                  FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  auto guard2 = make_handle_scope(handle2);

  if (handle1 == INVALID_HANDLE_VALUE && handle2 == INVALID_HANDLE_VALUE) {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
    return false;
  }
  else if (handle1 == INVALID_HANDLE_VALUE || handle2 == INVALID_HANDLE_VALUE) {
    // in case only one doesn't exist, return false
    return false;
  }

  auto info1 = BY_HANDLE_FILE_INFORMATION{};
  auto info2 = BY_HANDLE_FILE_INFORMATION{};

  if (!::GetFileInformationByHandle(handle1, &info1)) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return false;
  }
  if (!::GetFileInformationByHandle(handle2, &info2)) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return false;
  }

  return std::tie(info1.dwVolumeSerialNumber, info1.nFileIndexHigh, info1.nFileIndexLow,
                  info1.nFileSizeHigh, info1.nFileSizeLow,
                  info1.ftLastWriteTime.dwLowDateTime,
                  info1.ftLastWriteTime.dwHighDateTime)
         == std::tie(info2.dwVolumeSerialNumber, info2.nFileIndexHigh,
                     info2.nFileIndexLow, info2.nFileSizeHigh, info2.nFileSizeLow,
                     info2.ftLastWriteTime.dwLowDateTime,
                     info2.ftLastWriteTime.dwHighDateTime);
}


file_size_type
file_size(const path& p, std::error_code& ec) NOEXCEPT
{
  auto handle =
    ::CreateFileW(p.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
                  nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle != INVALID_HANDLE_VALUE) {
    auto guard = make_handle_scope(handle);

    auto size = LARGE_INTEGER{};
    if (::GetFileSizeEx(handle, &size)) {
      ec.clear();
      return static_cast<file_size_type>(size.QuadPart);
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
  return static_cast<file_size_type>(-1);
}


std::uintmax_t
hard_link_count(const path& p, std::error_code& ec) NOEXCEPT
{
  auto handle =
    ::CreateFileW(p.c_str(), FILE_READ_ATTRIBUTES,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle != INVALID_HANDLE_VALUE) {
    auto guard = make_handle_scope(handle);

    auto file_info = BY_HANDLE_FILE_INFORMATION{};
    if (::GetFileInformationByHandle(handle, &file_info)) {
      ec.clear();
      return file_info.nNumberOfLinks;
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
  return static_cast<file_size_type>(-1);
}


file_time_type
last_write_time(const path& p, std::error_code& ec) NOEXCEPT
{
  auto handle =
    ::CreateFileW(p.c_str(), FILE_READ_ATTRIBUTES,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle != INVALID_HANDLE_VALUE) {
    auto guard = make_handle_scope(handle);

    auto last_write_time = FILETIME{};
    if (::GetFileTime(handle, nullptr, nullptr, &last_write_time)) {
      auto ti = ULARGE_INTEGER{};
      ti.LowPart = last_write_time.dwLowDateTime;
      ti.HighPart = last_write_time.dwHighDateTime;

      ec.clear();
      return ti.QuadPart;
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
  return 0;  // std::numeric_limits<file_time_type>::min();
}


void
last_write_time(const path& p, file_time_type new_time, std::error_code& ec) NOEXCEPT
{
  auto handle =
    ::CreateFileW(p.c_str(), FILE_WRITE_ATTRIBUTES,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle != INVALID_HANDLE_VALUE) {
    auto guard = make_handle_scope(handle);

    auto ti = ULARGE_INTEGER{};
    ti.QuadPart = new_time;
    auto last_write_time = FILETIME{ti.LowPart, ti.HighPart};
    if (::SetFileTime(handle, nullptr, nullptr, &last_write_time)) {
      ec.clear();
      return;
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
}


path
read_symlink(const path& p, std::error_code& ec) NOEXCEPT
{
  return read_reparse_point(p, ec);
}


bool
remove(const path& p, std::error_code& ec) NOEXCEPT
{
  DWORD attr;
  auto ty = symlink_file_type(p, attr, ec);
  if (ec) {
    return false;
  }

  if (ty == file_type_impl::directory || ty == file_type_impl::symlink_directory) {
    if (::RemoveDirectoryW(p.c_str())) {
      ec.clear();
      return true;
    }
  }
  else {
    if (::DeleteFileW(p.c_str())) {
      ec.clear();
      return true;
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
  return false;
}


std::uintmax_t
remove_all(const path& p, std::error_code& ec) NOEXCEPT
{
  using std::end;

  std::uintmax_t count = 0;
  auto iter = directory_iterator(p, ec);
  if (ec) {
    return count;
  }

  for (; iter != end(iter); ++iter) {
    auto& e = *iter;

    DWORD attr;
    auto ty = symlink_file_type(e.path(), attr, ec);
    if (ec) {
      return count;
    }

    if (ty == file_type_impl::directory) {
      count += impl::remove_all(e.path(), ec);
      if (ec) {
        return count;
      }
    }
    else {
      impl::remove(e.path(), ec);
      if (ec) {
        return count;
      }
      ++count;
    }
  }

  impl::remove(p, ec);
  if (ec) {
    return count;
  }

  ec.clear();
  ++count;
  return count;
}


void
permissions(const path& p, perms prms, std::error_code& ec) NOEXCEPT
{
  if ((prms & perms::add_perms) != 0 && (prms & perms::remove_perms) != 0) {
    ec = std::error_code(EINVAL, std::generic_category());
    return;
  }

  // Since windows doesn't support the posix permissions, we only interpret
  // the READONLY flag.
  if ((prms & (perms::add_perms | perms::remove_perms)) != 0) {
    if ((prms & (perms::owner_write | perms::group_write | perms::others_write)) != 0) {
      return;
    }
  }

  auto target_p =
    (prms & perms::resolve_symlinks) != 0 ? get_path(p, ec) : get_symlink_path(p, ec);
  if (ec) {
    return;
  }

  auto attr = ::GetFileAttributesW(target_p.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return;
  }

  if ((prms & perms::add_perms) != 0) {
    attr &= ~FILE_ATTRIBUTE_READONLY;
  }
  else if ((prms & perms::remove_perms) != 0) {
    attr |= FILE_ATTRIBUTE_READONLY;
  }
  else if ((prms & (perms::owner_write | perms::group_write | perms::others_write))
           != 0) {
    attr &= ~FILE_ATTRIBUTE_READONLY;
  }
  else {
    attr |= FILE_ATTRIBUTE_READONLY;
  }

  if (!::SetFileAttributesW(target_p.c_str(), attr)) {
    ec = std::error_code(::GetLastError(), std::system_category());
  }
  else {
    ec.clear();
  }
}


void
rename(const path& old_p, const path& new_p, std::error_code& ec) NOEXCEPT
{
  if (!::MoveFileW(old_p.c_str(), new_p.c_str())) {
    ec = std::error_code(::GetLastError(), std::system_category());
  }
  else {
    ec.clear();
  }
}


void
resize_file(const path& p, file_size_type new_size, std::error_code& ec) NOEXCEPT
{
  auto handle =
    ::CreateFileW(p.c_str(), GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                  OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle != INVALID_HANDLE_VALUE) {
    auto guard = make_handle_scope(handle);

    auto ofs = LARGE_INTEGER{};
    ofs.QuadPart = new_size;
    if (::SetFilePointerEx(handle, ofs, nullptr, FILE_BEGIN)) {
      if (::SetEndOfFile(handle)) {
        ec.clear();
        return;
      }
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
}


space_info
space(const path& p, std::error_code& ec) NOEXCEPT
{
  space_info result;

  DWORD sectors_per_cluster = 0;
  DWORD bytes_per_sector = 0;
  DWORD number_of_free_clusters = 0;
  DWORD total_number_of_clusters = 0;
  if (!::GetDiskFreeSpaceW(p.c_str(), &sectors_per_cluster, &bytes_per_sector,
                           &number_of_free_clusters, &total_number_of_clusters)) {
    result.capacity = static_cast<std::uintmax_t>(-1);
    result.free = static_cast<std::uintmax_t>(-1);
    result.available = static_cast<std::uintmax_t>(-1);

    ec = std::error_code(::GetLastError(), std::system_category());
    return result;
  }

  result.capacity = (total_number_of_clusters * sectors_per_cluster * bytes_per_sector);
  result.free = (number_of_free_clusters * sectors_per_cluster * bytes_per_sector);
  result.available = static_cast<std::uintmax_t>(-1);

  ec.clear();
  return result;
}


file_status
status(const path& p, std::error_code& ec) NOEXCEPT
{
  int loop_cnt = 0;
  auto q = p;
  while (loop_cnt < 31) {
    auto attr = ::GetFileAttributesW(q.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
      auto err = ::GetLastError();
      if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
        ec.clear();
        return file_status(file_type::not_found);
      }

      ec = std::error_code(err, std::system_category());
      return file_status(file_type::none);
    }

    // GetFileAttributesW behave actually like lstat, i.e. if p refers to a
    // symlink it returns the data for the symlink itself; try follow it.
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) {
      auto res = read_reparse_point(q, ec);
      if (ec) {
        return file_status(file_type::none);
      }
      if (!res.empty()) {
        q = res;
        ++loop_cnt;
        continue;
      }
    }

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
      ec.clear();
      return file_status(file_type::directory, guess_permissions(q, attr));
    }

    ec.clear();
    return file_status(file_type::regular, guess_permissions(q, attr));
  }

  ec = std::make_error_code(std::errc::too_many_symbolic_link_levels);
  return file_status(file_type::none);
}


file_status
symlink_status(const path& p, std::error_code& ec) NOEXCEPT
{
  DWORD attr;
  auto ty = symlink_file_type(p, attr, ec);
  if (ec) {
    return file_status(file_type::none);
  }

  if (ty == file_type_impl::symlink_directory) {
    return file_status(file_type::symlink, guess_permissions(p, attr));
  }
  else {
    return file_status(static_cast<file_type>(ty), guess_permissions(p, attr));
  }
}

}  // impl


path
current_path(std::error_code& ec) NOEXCEPT
{
  auto charsReq = ::GetCurrentDirectoryW(0, nullptr);
  if (charsReq > 0) {
    auto buffer = std::vector<wchar_t>(charsReq + 1);
    if (::GetCurrentDirectoryW(charsReq + 1, buffer.data()) > 0) {
      ec.clear();
      return path(begin(buffer), end(buffer));
    }
  }

  ec = std::error_code(::GetLastError(), std::system_category());
  return {};
}


void
current_path(const path& p, std::error_code& ec) NOEXCEPT
{
  if (!::SetCurrentDirectoryW(p.c_str())) {
    ec = std::error_code(::GetLastError(), std::system_category());
  }
  else {
    ec.clear();
  }
}


path
system_complete(const path& p, std::error_code& ec) NOEXCEPT
{
  if (p.empty() || p.is_absolute()) {
    ec.clear();
    return p;
  }
  else {
    auto buffer = std::array<wchar_t, 258>{};

    auto len = ::GetFullPathNameW(
      p.c_str(), static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
    if (len < buffer.size()) {
      return path{buffer.data()};
    }
    else {
      // try again with a larger buffer
      auto buffer2 = std::vector<wchar_t>(len);
      len = ::GetFullPathNameW(
        p.c_str(), static_cast<DWORD>(buffer2.size()), buffer.data(), nullptr);
      if (len < buffer2.size()) {
        ec = std::make_error_code(std::errc::filename_too_long);
        return {};
      }

      return path{buffer2.data()};
    }
  }
}


path
temp_directory_path(std::error_code& ec) NOEXCEPT
{
  wchar_t buffer[MAX_PATH + 1];
  const auto len = ::GetTempPathW(MAX_PATH, buffer);
  if (len == 0) {
    ec = std::error_code(::GetLastError(), std::system_category());
    return {};
  }

  const auto p = path(buffer, buffer + len);
  const auto fs = status(p, ec);
  if (ec) {
    return {};
  }

  if (fs.type() == file_type::not_found) {
    ec = std::make_error_code(std::errc::no_such_file_or_directory);
    return {};
  }

  if (is_directory(fs)) {
    ec.clear();
    return p;
  }

  ec = std::make_error_code(std::errc::not_a_directory);
  return {};
}

}  // namespace filesystem
}  // namespace eyestep
