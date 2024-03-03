#pragma once

#include "constr.hpp"

namespace semitone
{
  class sat_eq final : public constr
  {
  public:
    sat_eq(sat_core &s, const utils::lit &l, const utils::lit &r, const utils::lit &e);

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const utils::lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    utils::lit left;  // The left-hand side of the equality.
    utils::lit right; // The right-hand side of the equality.
    utils::lit ctr;   // The reified equality.
  };
} // namespace semitone
