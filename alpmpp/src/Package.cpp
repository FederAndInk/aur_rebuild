#include <alpmpp/Package.hpp>

#include <alpmpp/Handle.hpp>

#include "alpmpp_utils.hpp"

namespace alpmpp
{
[[nodiscard]] std::vector<Package> Package::get_depends(Handle& handle) const
  noexcept(false)
{
  return list_to_vector<Package, alpm_depend_t*>(
    alpm_pkg_get_depends(pkg),
    [&handle, this](alpm_depend_t* dep) { return handle.get_local_pkg(dep->name); });
}

[[nodiscard]] std::vector<Package> Package::get_optdepends(Handle& handle) const
  noexcept(false)
{
  return list_to_vector<Package, alpm_depend_t*>(
    alpm_pkg_get_optdepends(pkg),
    [&handle, this](alpm_depend_t* dep) { return handle.get_local_pkg(dep->name); });
}

void Package::init_from_provider(std::string_view name, alpm_db_t* db) noexcept
{
  auto*      pkg_as_dep = alpm_dep_from_string(name.data());
  auto const pkg_hash = pkg_as_dep->name_hash;
  alpm_dep_free(pkg_as_dep);
  auto* pkgs = alpm_db_get_pkgcache(db);
  for (auto* p : Alpm_range<alpm_pkg_t*>{pkgs})
  {
    for (auto* dep : Alpm_range<alpm_depend_t*>{alpm_pkg_get_provides(p)})
    {
      if (pkg_hash == dep->name_hash && name == dep->name)
      {
        pkg = p;
        return;
      }
    }
  }
}

void Package::init_orig_db(Alpm_range<alpm_db_t*> sync_dbs) noexcept
{
  if (is_valid())
  {
    for (auto* sync_db : sync_dbs)
    {
      auto* sync_pkg = alpm_db_get_pkg(sync_db, get_name().data());
      if (sync_pkg)
      {
        orig_db = alpm_db_get_name(sync_db);
        return;
      }
    }
  }
}
} // namespace alpmpp