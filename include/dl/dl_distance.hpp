#pragma once

#include "lit.hpp"

namespace semitone
{
  template <typename T>
  class dl_distance
  {
  public:
    dl_distance(const utils::lit &b, VARIABLE_TYPE from, VARIABLE_TYPE to, const T &dist) noexcept : b(b), from(from), to(to), dist(dist) {}

    [[nodiscard]] const utils::lit &get_lit() const noexcept { return b; }
    [[nodiscard]] VARIABLE_TYPE get_from() const noexcept { return from; }
    [[nodiscard]] VARIABLE_TYPE get_to() const noexcept { return to; }
    [[nodiscard]] const T &get_dist() const noexcept { return dist; }

  private:
    utils::lit b;
    VARIABLE_TYPE from;
    VARIABLE_TYPE to;
    T dist;
  };
} // namespace semitone
