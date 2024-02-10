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
    clause(sat_core &s, std::vector<lit> &&ls);
    ~clause();

    /**
     * @brief Get the literals of the clause.
     *
     * @return const std::vector<lit>& the literals of the clause.
     */
    const std::vector<lit> &get_lits() const { return lits; }

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<lit> get_reason(const lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    std::vector<lit> lits;
  };
} // namespace semitone
