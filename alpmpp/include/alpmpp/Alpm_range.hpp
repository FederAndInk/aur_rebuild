#pragma once

#include <alpm_list.h>

#include <iterator>

namespace alpmpp
{

template <typename T>
class Alpm_range
{
  alpm_list_t* list;
  alpm_list_t  lend;

public:
  explicit(false) Alpm_range(alpm_list_t* list) noexcept
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

    explicit Iterator(alpm_list_t* list, alpm_list_t* lend) noexcept
      : list{list}, lend{lend}
    {
    }

    reference operator*() const noexcept
    {
      return *(this->operator->());
    }

    pointer operator->() const noexcept
    {
      return reinterpret_cast<pointer>(&list->data);
    }

    Iterator& operator++() noexcept
    {
      list = list->next;
      return *this;
    }

    Iterator operator++(int) const noexcept
    {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    Iterator& operator--() noexcept
    {
      list = list->prev;
      return *this;
    }

    Iterator operator--(int) const noexcept
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

  [[nodiscard]] Iterator begin() noexcept
  {
    return Iterator{list, &lend};
  }

  [[nodiscard]] Iterator end() noexcept
  {
    return Iterator{&lend, &lend};
  }
};
} // namespace alpmpp
