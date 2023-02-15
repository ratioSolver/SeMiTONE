#pragma once

#include "constr.h"

namespace semitone
{
  class sat_core;
  class theory;

  /**
   * This class is used for representing propositional clauses.
   */
  class clause final : public constr
  {
    friend class sat_core;
    friend class theory;

  private:
    /**
     * @brief Construct a new clause object given the `lits` literals.
     *
     * @param s the sat core.
     * @param lits the literals of the clause.
     */
    clause(sat_core &s, std::vector<lit> ls);
    clause(const clause &orig) = delete;
    ~clause();

  public:
    inline const std::vector<lit> get_lits() const noexcept { return lits; }

  private:
    constr *copy(sat_core &s) noexcept override { return new clause(s, lits); }
    bool propagate(const lit &p) noexcept override;
    bool simplify() noexcept override;
    void get_reason(const lit &p, std::vector<lit> &out_reason) const noexcept override;

    virtual json::json to_json() const noexcept override;

  private:
    std::vector<lit> lits;
  };
} // namespace semitone