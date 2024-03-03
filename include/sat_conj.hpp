#pragma once

#include "constr.hpp"

namespace semitone
{
  class sat_conj final : public constr
  {
  public:
    sat_conj(sat_core &s, std::vector<utils::lit> &&ls, const utils::lit &ctr);

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const utils::lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    std::vector<utils::lit> lits; // The literals of the conjunction.
    utils::lit ctr;               // The reified conjunction.
  };
} // namespace semitone
