#include <alpmpp/Handle.hpp>
#include <alpmpp/Package.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std::literals;

int main()
{
  alpmpp::Handle alpmh("/", "/var/lib/pacman", "/etc/pacman.conf");

  alpmpp::Package pkg = alpmh.get_local_pkg("gazebo");
  std::cout << pkg.get_name() << " version: " << pkg.get_version() << "\n";
  std::cout << "nb deps: " << pkg.get_depends(alpmh).size() << "\n";
  std::cout << "nb optdeps: " << pkg.get_optdepends(alpmh).size() << "\n";
  // pkg = alpmh.get_pkg("opencv");
  // std::cout << pkg.get_name() << " version: " << pkg.get_version() << "\n";
  auto deps = pkg.get_depends(alpmh);
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
  auto optdeps = pkg.get_optdepends(alpmh);
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
