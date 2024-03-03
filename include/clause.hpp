#pragma once

#include "constr.hpp"

namespace semitone
{
  /**
   * This class is used for representing propositional clauses.
   */
  class clause final : public constr
  {
  public:
    /**
     * @brief Construct a new clause object given the `lits` literals.
     *
     * @param s the sat core.
     * @param lits the literals of the clause.
     */
    clause(sat_core &s, std::vector<utils::lit> &&ls);

    /**
     * @brief Get the literals of the clause.
     *
     * @return const std::vector<lit>& the literals of the clause.
     */
    const std::vector<utils::lit> &get_lits() const { return lits; }

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const utils::lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    std::vector<utils::lit> lits;
  };
} // namespace semitone
