#include <alpm.h>
#include <alpm_list.h>

#include <iostream>
#include <vector>

using namespace std::literals;

namespace alpm
{

template <typename T>
class Alpm_range
{
  alpm_list_t* list;
  alpm_list_t  lend;

public:
  explicit(false) Alpm_range(alpm_list_t* list)
    : list(list), lend{.data = nullptr, .prev = alpm_list_last(list), .next = nullptr}
  {
  }

  class Iterator
  {
    alpm_list_t* list;
    alpm_list_t* lend;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type*;   // or also value_type*
    using reference = value_type&; // or also value_type&

    explicit Iterator(alpm_list_t* list, alpm_list_t* lend) : list{list}, lend{lend} {}

    reference operator*() const
    {
      return *(this->operator->());
    }

    pointer operator->() const
    {
      return reinterpret_cast<pointer>(&list->data);
    }

    Iterator& operator++()
    {
      list = list->next;
      return *this;
    }

    Iterator operator++(int) const
    {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    Iterator& operator--()
    {
      list = list->prev;
      return *this;
    }

    Iterator operator--(int) const
    {
      auto tmp = *this;
      --(*this);
      return tmp;
    }

    friend bool operator==(Iterator const& lhs, Iterator const& rhs) noexcept
    {
      return lhs.list == rhs.list || (lhs.lend == rhs.list && lhs.list == nullptr) ||
             (lhs.list == rhs.lend && rhs.list == nullptr);
    }

    friend bool operator!=(Iterator const& lhs, Iterator const& rhs) noexcept
    {
      return !(lhs == rhs);
    }
  };

  [[nodiscard]] Iterator begin()
  {
    return Iterator{list, &lend};
  }

  [[nodiscard]] Iterator end()
  {
    return Iterator{&lend, &lend};
  }
};

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
  Package(alpm_db_t* db, std::string_view name)
  {
    pkg = alpm_db_get_pkg(db, name.data());
    if (!is_valid())
    {
      auto* pkg_as_dep = alpm_dep_from_string(name.data());
      auto  pkg_hash = pkg_as_dep->name_hash;
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
  Package get_pkg(std::string_view name) const
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
  // pkg = alpmh.get_pkg("opencv");
  // std::cout << pkg.get_name() << " version: " << pkg.get_version() << "\n";
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
      std::cout << "  " << dep.get_name();
      if (dep.get_installdate() > pkg.get_builddate())
      {
        std::cout << " newer than pkg!";
      }
      std::cout << "\n";
    }
    else
    {
      std::cout << "  invalid optdep\n";
    }
  }
}
