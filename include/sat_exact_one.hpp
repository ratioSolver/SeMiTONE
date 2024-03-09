#pragma once

#include "constr.hpp"

namespace semitone
{
  class sat_exact_one final : public constr
  {
  public:
    sat_exact_one(sat_core &s, std::vector<utils::lit> &&ls, const utils::lit &ctr) noexcept;

  private:
    [[nodiscard]] std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    [[nodiscard]] bool simplify() noexcept override;

    [[nodiscard]] std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept override;

    [[nodiscard]] json::json to_json() const noexcept override;

  private:
    std::vector<utils::lit> lits; // The literals of the exact-one.
    utils::lit ctr;               // The reified exact-one.
  };
} // namespace semitone
