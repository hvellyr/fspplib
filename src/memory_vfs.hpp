// Copyright (c) 2016 Gregor Klinke

#pragma once

#include "fspp/details/config.hpp"

#include "fspp/details/dir_iterator.hpp"
#include "fspp/details/file.hpp"
#include "fspp/details/vfs.hpp"
#include "fspp/filesystem.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>


namespace eyestep {
namespace filesystem {
namespace vfs {

class FSNode
{
public:
  FSNode() = default;
  explicit FSNode(file_type type)
    : _type(type)
  {
  }

  FSNode(const FSNode& other) = default;
  FSNode& operator=(const FSNode& other) = default;

  void set_data(std::string data)
  {
    _data = std::move(data);
    _file_size = _data.size();
    touch();
  }

  void copy_from_other(const FSNode& other)
  {
    _data = other._data;
    _file_size = other._file_size;
    _last_write_time = other._last_write_time;
  }

  void touch()
  {
    _last_write_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  }

  file_type _type = file_type::none;
  file_time_type _last_write_time;
  file_size_type _file_size = 0;
  perms _perms;
  // if _type == // directory_file
  std::unordered_map<std::string, std::shared_ptr<FSNode>> _children;
  // if _type == regular_file
  std::string _data;
};


class MemoryFilesystem : public vfs::IFilesystem
{
public:
  MemoryFilesystem();
  std::shared_ptr<FSNode> root_node();

  std::unique_ptr<detail::IFileImpl> make_file_impl() override;
  std::unique_ptr<directory_iterator::IDirIterImpl> make_dir_iterator(
    const path& p, std::error_code& ec) override;

  void dump(std::ostream& os) override;
  path canonical(const path& p, const path& base, std::error_code& ec) override;
  bool copy_file(const path& from,
                 const path& to,
                 copy_options options,
                 std::error_code& ec) override;
  void copy_symlink(const path& from, const path& to, std::error_code& ec) override;
  bool create_directory(const path& p, std::error_code& ec) override;
  bool create_directory(const path& p,
                        const path& existing_p,
                        std::error_code& ec) override;
  bool create_directories(const path& path, std::error_code& ec) override;
  void create_hard_link(const path& target,
                        const path& link,
                        std::error_code& ec) override;
  void create_symlink(const path& target, const path& link, std::error_code& ec) override;
  void create_directory_symlink(const path& target,
                                const path& link,
                                std::error_code& ec) override;
  bool equivalent(const path& p1, const path& p2, std::error_code& ec) override;
  file_size_type file_size(const path& p, std::error_code& ec) override;
  std::uintmax_t hard_link_count(const path& p, std::error_code& ec) override;
  file_time_type last_write_time(const path& p, std::error_code& ec) override;
  void last_write_time(const path& p,
                       file_time_type new_time,
                       std::error_code& ec) override;
  void permissions(const path& p, perms prms, std::error_code& ec) override;
  path read_symlink(const path& p, std::error_code& ec) override;
  bool remove(const path& p, std::error_code& ec) override;
  std::uintmax_t remove_all(const path& p, std::error_code& ec) override;
  void rename(const path& old_p, const path& new_p, std::error_code& ec) override;
  void resize_file(const path& p, file_size_type new_size, std::error_code& ec) override;
  space_info space(const path& p, std::error_code& ec) NOEXCEPT override;
  file_status status(const path& p, std::error_code& ec) NOEXCEPT override;
  file_status symlink_status(const path& p, std::error_code& ec) NOEXCEPT override;

private:
  std::shared_ptr<FSNode> _root;
};


//----------------------------------------------------------------------------------------

class MemoryVfsFileImpl : public detail::IFileImpl
{
  enum FileMode
  {
    k_not_open,
    k_read,
    k_write,
    k_readwrite
  };

  MemoryFilesystem* _fs;
  std::stringstream _stream;
  FileMode _filemode = k_not_open;
  std::shared_ptr<FSNode> _node;

public:
  explicit MemoryVfsFileImpl(MemoryFilesystem* fs);
  ~MemoryVfsFileImpl() override;

  std::iostream& open(const path& p,
                      std::ios::openmode mode,
                      std::error_code& ec) override;
  std::iostream& stream() override;
  bool is_open() const override;
  void close(std::error_code& ec) override;
};


/*! Removes . and .. */
path
canonicalize_memory_vfs_path(const path& p, std::error_code& ec);


}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep
