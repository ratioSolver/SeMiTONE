#pragma once

#include "lit.hpp"

namespace semitone
{
  template <typename T>
  class distance_constraint
  {
  public:
    distance_constraint(const utils::lit &b, std::size_t from, std::size_t to, const T &dist) noexcept : b(b), from(from), to(to), dist(dist) {}

    [[nodiscard]] const utils::lit &get_lit() const noexcept { return b; }
    [[nodiscard]] std::size_t get_from() const noexcept { return from; }
    [[nodiscard]] std::size_t get_to() const noexcept { return to; }
    [[nodiscard]] const T &get_dist() const noexcept { return dist; }

  private:
    utils::lit b;
    std::size_t from;
    std::size_t to;
    T dist;
  };
} // namespace semitone
