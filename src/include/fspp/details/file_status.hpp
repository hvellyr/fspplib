// Copyright (c) 2016 Gregor Klinke

#pragma once

#if defined(USE_FSPP_CONFIG_HPP)
#include "fspp-config.hpp"
#else
#include "fspp/details/fspp-config.hpp"
#endif

#include "fspp/details/platform.hpp"
#include "fspp/details/types.hpp"


namespace eyestep {
namespace filesystem {

/*! Stores information about the type and permissions of a file. */
class file_status
{
public:
  explicit file_status(file_type type = file_type::none,
                       perms permissions = perms::unknown)
    : _type(type)
    , _perms(permissions)
  {
  }

  file_status(const file_status&) = default;
#if defined(FSPP_IS_VS2013)
  file_status(file_status&& rhs)
    : _type(rhs._type)
    , _perms(rhs._perms)
  {
  }
#else
  file_status(file_status&&) = default;
#endif

  file_status& operator=(const file_status& rhs) = default;
#if defined(FSPP_IS_VS2013)
  file_status& operator=(file_status&& rhs)
  {
    _type = rhs._type;
    _perms = rhs._perms;
    return *this;
  }
#else
  file_status& operator=(file_status&& rhs) = default;
#endif

  /*! @returns the file type information. */
  file_type type() const { return _type; }
  /*! Sets file type to @p type. */
  void type(file_type type) { _type = type; }

  /*! @returns the file permissions information. */
  perms permissions() const { return _perms; }
  /*! Sets file permissions to @p perm. */
  void permissions(perms p) { _perms = p; }

private:
  file_type _type;
  perms _perms;
};

}  //  namespace filesystem
}  // namespace eyestep
