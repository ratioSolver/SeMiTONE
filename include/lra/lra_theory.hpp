#pragma once

#include <vector>
#include "theory.hpp"
#include "inf_rational.hpp"
#include "tableau.hpp"

namespace semitone
{
  class lra_theory final : public theory
  {
  public:
    lra_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Creates a new lower then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    utils::lit new_lt(const utils::lit &left, const utils::lit &right) noexcept;
    /**
     * @brief Creates a new lower or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    utils::lit new_leq(const utils::lit &left, const utils::lit &right) noexcept;
    /**
     * @brief Creates a new equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    utils::lit new_eq(const utils::lit &left, const utils::lit &right) noexcept;
    /**
     * @brief Creates a new greater or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    utils::lit new_geq(const utils::lit &left, const utils::lit &right) noexcept;
    /**
     * @brief Creates a new greater then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    utils::lit new_gt(const utils::lit &left, const utils::lit &right) noexcept;

  private:
    bool propagate(const utils::lit &) noexcept override { return true; }
    bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

  private:
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value; // the value of the bound..
      utils::lit reason;         // the reason for the value..
    };

    std::vector<bound> c_bounds;           // the current bounds..
    std::vector<utils::inf_rational> vals; // the current values..
    utils::tableau tableau;                // the tableau..
  };
} // namespace semitone
