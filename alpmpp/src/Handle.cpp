#include <alpmpp/Handle.hpp>

#include "alpmpp_utils.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace alpmpp
{
using namespace std::literals;
Handle::Handle(char const* root, char const* dbpath) noexcept(false)
{
  alpm_errno_t err;
  handle = alpm_initialize(root, dbpath, &err);
  if (!handle)
  {
    throw std::runtime_error("error: cannot initialize alpm: "s + alpm_strerror(err) +
                             '\n');
  }
}

Handle::Handle(char const* root, char const* dbpath, char const* confpath) noexcept(false)
  : Handle(root, dbpath)
{
  std::ifstream conf_file(confpath);
  if (!conf_file)
  {
    throw std::runtime_error("error: config file "s + confpath + " could not be read\n");
  }

  const alpm_siglevel_t level =
    static_cast<alpm_siglevel_t>(ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL);
  std::string line;
  while (std::getline(conf_file, line))
  {
    line.erase(std::ranges::find(line, '#'), std::end(line));
    std::string_view tline = trim(line);

    if (tline.empty())
    {
      continue;
    }

    if (tline.front() == '[' && tline.back() == ']')
    {
      tline.remove_prefix(1);
      tline.remove_suffix(1);

      if (tline != "options")
      {
        line = tline;
        alpm_db_t* db = alpm_register_syncdb(handle, line.c_str(), level);
        alpm_db_set_usage(db, ALPM_DB_USAGE_ALL);
      }
    }
  }
}

Handle::~Handle() noexcept
{
  alpm_release(handle);
}

/**
 * @brief get a package from local database
 *
 */
[[nodiscard]] Package Handle::get_local_pkg(std::string_view name) const noexcept
{
  return Package(name, alpm_get_localdb(handle), alpm_get_syncdbs(handle));
}

} // namespace alpmpp