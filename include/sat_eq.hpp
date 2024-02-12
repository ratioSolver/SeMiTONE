#pragma once

#include "constr.hpp"

namespace semitone
{
  class sat_eq final : public constr
  {
  public:
    sat_eq(sat_core &s, const lit &l, const lit &r, const lit &e);

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<lit> get_reason(const lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    lit left;  // The left-hand side of the equality.
    lit right; // The right-hand side of the equality.
    lit ctr;   // The reified equality.
  };
} // namespace semitone
