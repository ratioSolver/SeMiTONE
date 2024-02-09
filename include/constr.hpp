#pragma once

#include <vector>
#include "lit.hpp"
#include "bool.hpp"

namespace semitone
{
  class sat_core;

  /**
   * This class is used for representing propositional constraints.
   */
  class constr
  {
  public:
    constr(sat_core &s) : sat(s) {}
    virtual ~constr() = default;

  private:
    /**
     * Propagate a literal through the constraint.
     *
     * @param p The literal to propagate.
     * @return `true` if the constraint network is consistent, `false` otherwise.
     */
    virtual bool propagate(const lit &p) noexcept = 0;
    /**
     * Check if the constraint is redundant under the current assignment.
     *
     * @return `true` if the constraint is redundant, and can be removed from the network, `false` otherwise.
     */
    virtual bool simplify() noexcept = 0;

  protected:
    /**
     * Compute the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    utils::lbool value(const VARIABLE_TYPE &x) const noexcept;
    /**
     * Compute the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    utils::lbool value(const lit &p) const noexcept;

    void remove_constr_from_reason(const VARIABLE_TYPE &x) noexcept;

  protected:
    sat_core &sat;
  };
} // namespace semitone
