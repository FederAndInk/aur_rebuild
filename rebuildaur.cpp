#include <alpmpp/Handle.hpp>
#include <alpmpp/Package.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::literals;

struct string_hash
{
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  std::size_t operator()(const char* str) const
  {
    return hash_type{}(str);
  }
  std::size_t operator()(std::string_view str) const
  {
    return hash_type{}(str);
  }
  std::size_t operator()(std::string const& str) const
  {
    return hash_type{}(str);
  }
};

class Dependecy_checker
{
  alpmpp::Handle&                      handle;
  std::unordered_set<std::string_view> excludes;

  std::unordered_map<std::string, bool, string_hash, std::equal_to<>> seen;

public:
  explicit Dependecy_checker(alpmpp::Handle&                      handle,
                             std::unordered_set<std::string_view> excludes)
    : handle(handle), excludes(std::move(excludes))
  {
  }

  void reset()
  {
    seen.clear();
  }

  enum class Lvl
  {
    NONE,
    EXPLAIN
  };
  template <Lvl explain = Lvl::NONE>
  bool check_deps_explain(alpmpp::Package& pkg)
  {
    std::string name{pkg.get_name()};
    if (auto it = seen.find(name); it != std::end(seen))
    {
      return it->second;
    }
    if (!pkg.get_orig_db().empty())
    {
      return false;
    }
    auto deps = pkg.get_depends(handle);
    bool rebuild = false;
    seen[name] = rebuild;

    std::vector<std::string_view> why;
    for (auto&& dep : deps)
    {
      if (!excludes.contains(dep.get_name()) &&
          dep.get_installdate() > pkg.get_builddate())
      {
        rebuild = true;
        if constexpr (explain == Lvl::EXPLAIN)
        {
          why.push_back(dep.get_name());
        }
      }
      seen.at(name) = rebuild;
      auto cdeps_ret = check_deps_explain<explain>(dep);
      rebuild = rebuild || cdeps_ret;
    }

    if (rebuild)
    {
      std::cout << pkg.get_name();
      if constexpr (explain == Lvl::EXPLAIN)
      {
        std::cout << " caused by: ";
        if (why.empty())
        {
          std::cout << "^";
        }
      }
      std::cout << "\n";
      if constexpr (explain == Lvl::EXPLAIN)
      {
        for (auto name : why)
        {
          std::cout << "  " << name << "\n";
        }
      }
    }

    return rebuild;
  }
};

void usage(std::string_view exe_name)
{
  std::cerr << "usage: " << exe_name << " pkgname\n";
  std::cerr << "options:\n";
  std::cerr << "  --why  : explain why the packages need to be rebuilt\n";
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    usage(argv[0]);
    std::exit(1);
  }
  bool explain = false;

  std::unordered_set<std::string_view> excludes;
  for (int i = 2; i < argc; ++i)
  {
    std::string_view arg = argv[i];
    if (arg.starts_with("--"))
    {
      if (arg == "--why"sv)
      {
        explain = true;
      }
      else if (arg.size() > 2)
      {
        std::cerr << "unknown option " << arg << "\n";
        usage(argv[0]);
        std::exit(1);
      }
    }
    else
    {
      excludes.insert(arg);
    }
  }

  alpmpp::Handle handle("/", "/var/lib/pacman", "/etc/pacman.conf");

  alpmpp::Package pkg = handle.get_local_pkg(argv[1]);
  if (!pkg.is_valid())
  {
    std::cerr << argv[1] << " is not a local package\n";
    std::exit(2);
  }
  Dependecy_checker dcheck{handle, std::move(excludes)};
  if (explain)
  {
    dcheck.check_deps_explain<Dependecy_checker::Lvl::EXPLAIN>(pkg);
  }
  else
  {
    dcheck.check_deps_explain<Dependecy_checker::Lvl::NONE>(pkg);
  }
}
