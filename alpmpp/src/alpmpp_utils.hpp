#pragma once

#include <alpm_list.h>

#include <algorithm>
#include <string_view>
#include <vector>

namespace alpmpp
{
inline std::string_view trim(std::string_view s)
{
  auto is_space = [](int c) { return std::isspace(c); };
  auto wsfront = std::find_if_not(s.begin(), s.end(), is_space);
  auto wsback = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
  if (wsback > wsfront)
  {
    return std::string_view(wsfront, wsback);
  }
  else
  {
    return {};
  }
}

template <typename T,
          typename Lt = T,
          typename Fn /* = decltype([](Lt const& lt) { return lt; }) */>
std::vector<T> list_to_vector(alpm_list_t* list, Fn fn = Fn{}) noexcept(false)
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

}; // namespace alpmpp
