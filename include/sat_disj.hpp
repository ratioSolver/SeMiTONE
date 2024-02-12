#pragma once

#include "constr.hpp"

namespace semitone
{
  class sat_disj final : public constr
  {
  public:
    sat_disj(sat_core &s, std::vector<lit> &&ls, const lit &ctr);

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<lit> get_reason(const lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    std::vector<lit> lits; // The literals of the disjunction.
    lit ctr;               // The reified disjunction.
  };
} // namespace semitone
