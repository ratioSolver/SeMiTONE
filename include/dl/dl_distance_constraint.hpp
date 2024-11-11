#pragma once

#include "lit.hpp"

namespace semitone
{
  template <typename T>
  class distance_constraint
  {
  public:
    distance_constraint(const utils::lit &b, utils::var from, utils::var to, const T &dist) noexcept : b(b), from(from), to(to), dist(dist) {}

    [[nodiscard]] const utils::lit &get_lit() const noexcept { return b; }
    [[nodiscard]] utils::var get_from() const noexcept { return from; }
    [[nodiscard]] utils::var get_to() const noexcept { return to; }
    [[nodiscard]] const T &get_dist() const noexcept { return dist; }

  private:
    utils::lit b;
    utils::var from;
    utils::var to;
    T dist;
  };
} // namespace semitone
