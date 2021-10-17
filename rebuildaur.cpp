#include <alpm.h>
#include <alpm_list.h>

#include <iostream>
#include <vector>

using namespace std::literals;

namespace alpm
{

template <typename T,
          typename Lt = T,
          typename Fn = decltype([](Lt const& lt) { return lt; })>
std::vector<T> list_to_vector(alpm_list_t* list, Fn fn = Fn{})
{
  std::vector<T> v;
  v.reserve(alpm_list_count(list));
  while (list)
  {
    v.push_back(fn(static_cast<Lt>(list->data)));
    list = list->next;
  }

  return v;
}

class Package
{
  alpm_pkg_t* pkg{nullptr};

public:
  Package(alpm_db_t* db, const char* name)
  {
    pkg = alpm_db_get_pkg(db, name);
  }

  [[nodiscard]] bool is_valid() const
  {
    return pkg != nullptr;
  }

  [[nodiscard]] std::string_view get_name() const
  {
    return alpm_pkg_get_name(pkg);
  }

  [[nodiscard]] std::string_view get_version() const
  {
    return alpm_pkg_get_version(pkg);
  }

  [[nodiscard]] std::vector<Package> get_depends() const
  {
    return list_to_vector<Package, alpm_depend_t*>(
      alpm_pkg_get_depends(pkg),
      [this](alpm_depend_t* dep) { return Package(alpm_pkg_get_db(pkg), dep->name); });
  }

  [[nodiscard]] std::vector<Package> get_optdepends() const
  {
    return list_to_vector<Package, alpm_depend_t*>(
      alpm_pkg_get_optdepends(pkg),
      [this](alpm_depend_t* dep) { return Package(alpm_pkg_get_db(pkg), dep->name); });
    ;
  }

  // [[nodiscard]] std::string_view get_provides() const
  // {
  //   return alpm_pkg_get_provides(pkg);
  // }

  [[nodiscard]] alpm_time_t get_builddate() const
  {
    return alpm_pkg_get_builddate(pkg);
  }

  [[nodiscard]] alpm_time_t get_installdate() const
  {
    return alpm_pkg_get_installdate(pkg);
  }
};

class Handle
{
  alpm_handle_t* handle;

public:
  Handle(char const* root, char const* dbpath)
  {
    alpm_errno_t err;
    handle = alpm_initialize(root, dbpath, &err);
    if (!handle)
    {
      throw std::runtime_error("error: cannot initialize alpm: "s + alpm_strerror(err) +
                               '\n');
    }
  }

  ~Handle()
  {
    alpm_release(handle);
  }

  alpm_handle_t* get()
  {
    return handle;
  }

  /**
   * @brief get a package from local database
   *
   */
  Package get_pkg(char const* name) const
  {
    return Package(alpm_get_localdb(handle), name);
  }
};
} // namespace alpm

int main(int argc, char** argv)
{
  alpm::Handle alpmh("/", "/var/lib/pacman");

  alpm::Package pkg = alpmh.get_pkg("ros-noetic-desktop-full");
  std::cout << pkg.get_name() << " version: " << pkg.get_version() << "\n";
  std::cout << "nb deps: " << pkg.get_depends().size() << "\n";
  std::cout << "nb optdeps: " << pkg.get_optdepends().size() << "\n";
  pkg = alpmh.get_pkg("opencv");
  std::cout << pkg.get_name() << " version: " << pkg.get_version() << "\n";
  auto deps = pkg.get_depends();
  std::cout << "nb deps: " << deps.size() << "\n";
  for (auto&& dep : deps)
  {
    if (dep.is_valid())
    {
      std::cout << "  " << dep.get_name() << "\n";
    }
    else
    {
      std::cout << "  invalid dep!\n";
    }
  }
  auto optdeps = pkg.get_optdepends();
  std::cout << "nb optdeps: " << optdeps.size() << "\n";
  for (auto&& dep : optdeps)
  {
    if (dep.is_valid())
    {
      std::cout << "  " << dep.get_name() << "\n";
    }
    else
    {
      std::cout << "  invalid optdep\n";
    }
  }
}
