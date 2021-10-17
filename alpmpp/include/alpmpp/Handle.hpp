#pragma once

#include <alpm.h>

#include <alpmpp/Package.hpp>

namespace alpmpp
{
class Handle
{
  alpm_handle_t* handle{nullptr};

public:
  Handle(char const* root, char const* dbpath) noexcept(false);

  Handle(char const* root, char const* dbpath, char const* confpath) noexcept(false);

  ~Handle() noexcept;

  /**
   * @brief get a package from local database
   *
   */
  [[nodiscard]] Package get_local_pkg(std::string_view name) const noexcept;
};

} // namespace alpmpp