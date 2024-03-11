#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include "theory.hpp"
#include "inf_rational.hpp"
#include "lra_assertion.hpp"
#include "lra_eq.hpp"
#include "json.hpp"

namespace semitone
{
  class lra_theory final : public theory
  {
    friend class lra_assertion;
    friend class lra_leq;
    friend class lra_geq;
    friend class lra_eq;

  public:
    lra_theory(std::shared_ptr<sat_core> sat) noexcept;

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

    /**
     * @brief Returns whether the two linear expressions overlap. That is, whether the two linear expressions can have a common value.
     *
     * @param l0 the first linear expression.
     * @param l1 the second linear expression.
     * @return bool whether the two linear expressions overlap.
     */
    bool matches(const utils::lin &l0, const utils::lin &l1) const noexcept
    {
      const auto [l0_lb, l0_ub] = bounds(l0);
      const auto [l1_lb, l1_ub] = bounds(l1);
      return l0_ub >= l1_lb && l0_lb <= l1_ub; // the two intervals overlap..
    }

    /**
     * @brief Asserts that the lower bound of variable `x_i` is `val` and returns whether the assertion was successful.
     *
     * @param x_i the variable to assert the lower bound of.
     * @param val the lower bound to assert.
     * @param p the literal that caused the assertion.
     * @return bool whether the assertion was successful.
     */
    bool assert_lower(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept;
    /**
     * @brief Asserts that the upper bound of variable `x_i` is `val` and returns whether the assertion was successful.
     *
     * @param x_i the variable to assert the upper bound of.
     * @param val the upper bound to assert.
     * @param p the literal that caused the assertion.
     * @return bool whether the assertion was successful.
     */
    bool assert_upper(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept;

    /**
     * @brief Sets the lower bound of variable `x_i` to `val` and propagates the change, returning whether the propagation was successful.
     *
     * @param x_i the variable to set the lower bound of.
     * @param val the lower bound to set.
     * @param p the literal that caused the change.
     * @return bool whether the propagation was successful.
     */
    bool set_lb(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept;
    /**
     * @brief Sets the upper bound of variable `x_i` to `val` and propagates the change, returning whether the propagation was successful.
     *
     * @param x_i the variable to set the upper bound of.
     * @param val the upper bound to set.
     * @param p the literal that caused the change.
     * @return bool whether the propagation was successful.
     */
    bool set_ub(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept;
    /**
     * @brief Sets the value of variable `x_i` to `val` and propagates the change, returning whether the propagation was successful.
     *
     * @param x_i the variable to set the value of.
     * @param val the value to set.
     * @param p the literal that caused the change.
     * @return bool whether the propagation was successful.
     */
    bool set_eq(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept;

  private:
    [[nodiscard]] inline static size_t lb_index(const VARIABLE_TYPE v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    [[nodiscard]] inline static size_t ub_index(const VARIABLE_TYPE v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    bool is_basic(const VARIABLE_TYPE v) const noexcept { return tableau.count(v); }
    void update(const VARIABLE_TYPE x_i, const utils::inf_rational &v) noexcept;
    void pivot_and_update(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j, const utils::inf_rational &v) noexcept;
    void pivot(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j) noexcept;
    void new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept;

    [[nodiscard]] bool propagate(const utils::lit &) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

    [[nodiscard]] friend json::json to_json(const lra_theory &rhs) noexcept;

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
    std::unordered_map<std::string, VARIABLE_TYPE> exprs;                      // the expressions (string to numeric variable) for which already exist slack variables..
    std::unordered_map<std::string, utils::lit> s_asrts;                       // the assertions (string to literal) used for reducing the number of boolean variables..
    std::vector<std::unordered_map<size_t, bound>> layers;                     // we store the updated bounds..
  };
} // namespace semitone
