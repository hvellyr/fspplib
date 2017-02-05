// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/file_status.hpp"
#include "fspp/filesystem.hpp"


namespace eyestep {
namespace filesystem {
namespace impl {

bool
copy_file(const path& from,
          const path& to,
          copy_options options,
          std::error_code& ec) NOEXCEPT;

bool
create_directory(const path& p, std::error_code& ec) NOEXCEPT;
bool
create_directory(const path& p, const path& existing_p, std::error_code& ec) NOEXCEPT;

void
create_hard_link(const path& target, const path& link, std::error_code& ec) NOEXCEPT;

void
create_symlink(const path& target, const path& link, std::error_code& ec) NOEXCEPT;
void
create_directory_symlink(const path& target,
                         const path& link,
                         std::error_code& ec) NOEXCEPT;

bool
equivalent(const path& p1, const path& p2, std::error_code& ec) NOEXCEPT;

file_size_type
file_size(const path& p, std::error_code& ec) NOEXCEPT;

std::uintmax_t
hard_link_count(const path& p, std::error_code& ec) NOEXCEPT;

file_time_type
last_write_time(const path& p, std::error_code& ec) NOEXCEPT;
void
last_write_time(const path& p, file_time_type new_time, std::error_code& ec) NOEXCEPT;

void
permissions(const path& p, perms prms, std::error_code& ec) NOEXCEPT;

path
read_symlink(const path& p, std::error_code& ec) NOEXCEPT;

bool
remove(const path& p, std::error_code& ec) NOEXCEPT;
std::uintmax_t
remove_all(const path& p, std::error_code& ec) NOEXCEPT;

void
rename(const path& old_p, const path& new_p, std::error_code& ec) NOEXCEPT;

void
resize_file(const path& p, file_size_type new_size, std::error_code& ec) NOEXCEPT;

space_info
space(const path& p, std::error_code& ec) NOEXCEPT;

file_status
status(const path& p, std::error_code& ec) NOEXCEPT;

file_status
symlink_status(const path& p, std::error_code& ec) NOEXCEPT;

}  // namespace impl
}  // namespace filesystem
}  // namespace eyestep
