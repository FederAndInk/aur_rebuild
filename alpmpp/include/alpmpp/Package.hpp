#pragma once

#include <alpmpp/Alpm_range.hpp>

#include <alpm.h>

#include <string>
#include <string_view>
#include <vector>

namespace alpmpp
{
class Handle;

class Package
{
  alpm_pkg_t* pkg{nullptr};
  std::string orig_db;

public:
  Package(std::string_view name, alpm_db_t* db, Alpm_range<alpm_db_t*> sync_dbs) noexcept
  {
    pkg = alpm_db_get_pkg(db, name.data());
    if (!is_valid())
    {
      init_from_provider(name, db);
    }
    init_orig_db(sync_dbs);
  }

  [[nodiscard]] bool is_valid() const noexcept
  {
    return pkg != nullptr;
  }

  [[nodiscard]] std::string_view get_name() const noexcept
  {
    return alpm_pkg_get_name(pkg);
  }

  [[nodiscard]] std::string_view get_version() const noexcept
  {
    return alpm_pkg_get_version(pkg);
  }

  [[nodiscard]] std::vector<Package> get_depends(Handle& handle) const noexcept(false);

  [[nodiscard]] std::vector<Package> get_optdepends(Handle& handle) const noexcept(false);
  // [[nodiscard]] std::string_view get_provides() const
  // {
  //   return alpm_pkg_get_provides(pkg);
  // }

  [[nodiscard]] alpm_time_t get_builddate() const noexcept
  {
    return alpm_pkg_get_builddate(pkg);
  }

  [[nodiscard]] alpm_time_t get_installdate() const noexcept
  {
    return alpm_pkg_get_installdate(pkg);
  }

  [[nodiscard]] std::string_view get_orig_db() const noexcept
  {
    return orig_db;
  }

private:
  void init_from_provider(std::string_view name, alpm_db_t* db) noexcept;

  void init_orig_db(Alpm_range<alpm_db_t*> sync_dbs) noexcept;
};

} // namespace alpmpp