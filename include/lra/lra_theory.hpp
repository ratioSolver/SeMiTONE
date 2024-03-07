#pragma once

#include <vector>
#include <set>
#include "theory.hpp"
#include "inf_rational.hpp"
#include "lra_assertion.hpp"
#include "lra_eq.hpp"

namespace semitone
{
  class lra_theory final : public theory
  {
    friend class lra_assertion;

  public:
    lra_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Create a new linear real arithmetic variable and make it equal to the given linear expression.
     *
     * @param l the linear expression to make the new variable equal to.
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var(const utils::lin &&l) noexcept;

    /**
     * @brief Creates a new lower then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_lt(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new lower or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_leq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_eq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new greater or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_geq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new greater then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_gt(const utils::lin &left, const utils::lin &right) noexcept;

    /**
     * @brief Returns the current lower bound of variable `v`.
     *
     * @param v the variable to get the lower bound of.
     * @return utils::inf_rational the current lower bound of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational lb(const VARIABLE_TYPE v) const noexcept { return c_bounds[lb_index(v)].value; }
    /**
     * @brief Returns the current upper bound of variable `v`.
     *
     * @param v the variable to get the upper bound of.
     * @return utils::inf_rational the current upper bound of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational ub(const VARIABLE_TYPE v) const noexcept { return c_bounds[ub_index(v)].value; }
    /**
     * @brief Returns the current value of variable `v`.
     *
     * @param v the variable to get the value of.
     * @return utils::inf_rational the current value of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational value(const VARIABLE_TYPE v) const noexcept { return vals[v]; }

    /**
     * @brief Returns the current lower bound of linear expression `l`.
     *
     * @param l the linear expression to get the lower bound of.
     * @return utils::inf_rational the current lower bound of linear expression `l`.
     */
    [[nodiscard]] inline utils::inf_rational lb(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? lb(v) : ub(v)) * c;
      return b;
    }
    /**
     * @brief Returns the current upper bound of linear expression `l`.
     *
     * @param l the linear expression to get the upper bound of.
     * @return utils::inf_rational the current upper bound of linear expression `l`.
     */
    [[nodiscard]] inline utils::inf_rational ub(const utils::lin &l) const noexcept
    {
      utils::inf_rational b(l.known_term);
      for (const auto &[v, c] : l.vars)
        b += (is_positive(c) ? ub(v) : lb(v)) * c;
      return b;
    }
    /**
     * @brief Returns the current bounds of linear expression `l`.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the current bounds of linear expression `l`.
     */
    [[nodiscard]] inline std::pair<utils::inf_rational, utils::inf_rational> bounds(const utils::lin &l) const noexcept
    {
      utils::inf_rational c_lb(l.known_term);
      utils::inf_rational c_ub(l.known_term);
      for (const auto &[v, c] : l.vars)
      {
        c_lb += (is_positive(c) ? lb(v) : ub(v)) * c;
        c_ub += (is_positive(c) ? ub(v) : lb(v)) * c;
      }
      return std::make_pair(c_lb, c_ub);
    }
    /**
     * @brief Returns the current value of linear expression `l`.
     *
     * @param l the linear expression to get the value of.
     * @return utils::inf_rational the current value of linear expression `l`.
     */
    [[nodiscard]] inline utils::inf_rational value(const utils::lin &l) const
    {
      utils::inf_rational val(l.known_term);
      for (const auto &[v, c] : l.vars)
        val += value(v) * c;
      return val;
    }

  private:
    inline static size_t lb_index(const VARIABLE_TYPE v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    inline static size_t ub_index(const VARIABLE_TYPE v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    bool propagate(const utils::lit &) noexcept override { return true; }
    bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

    void new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept;

  private:
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value; // the value of the bound..
      utils::lit reason;         // the reason for the value..
    };

    std::vector<bound> c_bounds;                                               // the current bounds..
    std::vector<utils::inf_rational> vals;                                     // the current values..
    std::map<const VARIABLE_TYPE, std::unique_ptr<lra_assertion>> v_asrts;     // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const VARIABLE_TYPE, std::unique_ptr<lra_eq>> tableau;            // the tableau..
    std::vector<std::vector<std::reference_wrapper<lra_assertion>>> a_watches; // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::set<VARIABLE_TYPE>> t_watches;                            // for each variable `v`, a list of tableau rows watching `v`..
  };
} // namespace semitone
