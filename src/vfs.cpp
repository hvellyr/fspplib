// Copyright (c) 2016 Gregor Klinke

#include "fspp/details/vfs.hpp"

#include "vfs_private.hpp"

#include "fspp/details/path.hpp"
#include "fspp/details/platform.hpp"
#include "fspp/filesystem.hpp"

#if defined(FSPP_HAVE_STD_CODECVT)
#include <codecvt>
#endif
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>


namespace eyestep {
namespace filesystem {
namespace vfs {

namespace {
path::string_type
convert_to_native(const std::string& str)
{
#if defined(FSPP_IS_WIN)
  static_assert(sizeof(path::value_type) == 2, "");
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv16;
  return conv16.from_bytes(str);
#else
  static_assert(sizeof(path::value_type) == 1, "");
  return str;
#endif
}


using FilesystemMap = std::unordered_map<path::string_type, std::unique_ptr<IFilesystem>>;

FilesystemMap&
vfs_registry()
{
  static FilesystemMap registry;
  return registry;
}
}  // namespace


bool
is_vfs_root_name(const path::string_type& rootname) NOEXCEPT
{
  return rootname.size() > 2 && rootname[0] == '/' && rootname[1] == '/'
         && rootname[2] == '<';
}


IFilesystem*
find_vfs(const path::string_type& rootname) NOEXCEPT
{
  auto& registry = vfs_registry();
  auto i_found = registry.find(rootname);
  return i_found != registry.end() ? i_found->second.get() : nullptr;
}


path
deroot(const path& p) NOEXCEPT
{
  return p.root_directory() / p.relative_path();
}


void
register_vfs(const std::string& name, std::unique_ptr<IFilesystem> fs)
{
  assert(name.find("//<") == 0);

  auto& registry = vfs_registry();
  auto nativekey = convert_to_native(name);
  assert(registry.find(nativekey) == registry.end());

  registry[nativekey] = std::move(fs);
}


std::unique_ptr<IFilesystem>
unregister_vfs(const std::string& name)
{
  auto& registry = vfs_registry();
  auto i_find = registry.find(convert_to_native(name));
  if (i_find != end(registry)) {
    auto retv = std::move(i_find->second);
    registry.erase(i_find);
    return retv;
  }

  return {};
}

}  // namespace vfs
}  // namespace filesystem
}  // namespace eyestep
