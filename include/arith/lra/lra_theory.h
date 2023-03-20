#pragma once

#include "sat_core.h"
#include "theory.h"
#include "lin.h"
#include "inf_rational.h"
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

namespace semitone
{
  class lra_value_listener;
  class assertion;
  class row;

  class lra_theory : public theory
  {
    friend class lra_value_listener;
    friend class assertion;
    friend class row;

  public:
    SEMITONE_EXPORT lra_theory(sat_ptr sat);
    SEMITONE_EXPORT lra_theory(sat_ptr sat, const lra_theory &orig);
    lra_theory(const lra_theory &orig) = delete;
    SEMITONE_EXPORT virtual ~lra_theory();

    SEMITONE_EXPORT var new_var() noexcept;             // creates and returns a new numeric variable..
    SEMITONE_EXPORT var new_var(const lin &l) noexcept; // creates and returns a new numeric variable and makes it equal to the given linear expression..

    inline bool is_basic(const var &v) const noexcept { return tableau.count(v); }

    /**
     * @brief Creates a new lower then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_lt(const lin &left, const lin &right) noexcept;
    /**
     * @brief Creates a new lower or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_leq(const lin &left, const lin &right) noexcept;
    /**
     * @brief Creates a new equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_eq(const lin &left, const lin &right) noexcept { return sat->new_conj({new_geq(left, right), new_leq(left, right)}); }
    /**
     * @brief Creates a new greater or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_geq(const lin &left, const lin &right) noexcept;
    /**
     * @brief Creates a new greater then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_gt(const lin &left, const lin &right) noexcept;

    /**
     * @brief Returns the current lower bound of variable `v`.
     *
     * @param v the variable to get the lower bound of.
     * @return utils::inf_rational the current lower bound of variable `v`.
     */
    inline utils::inf_rational lb(const var &v) const noexcept { return c_bounds[lb_index(v)].value; }
    /**
     * @brief Returns the current upper bound of variable `v`.
     *
     * @param v the variable to get the upper bound of.
     * @return utils::inf_rational the current upper bound of variable `v`.
     */
    inline utils::inf_rational ub(const var &v) const noexcept { return c_bounds[ub_index(v)].value; }
    /**
     * @brief Returns the current value of variable `v`.
     *
     * @param v the variable to get the value of.
     * @return utils::inf_rational the current value of variable `v`.
     */
    inline utils::inf_rational value(const var &v) const noexcept { return vals[v]; }

    /**
     * @brief Returns the current lower bound of linear expression `l`.
     *
     * @param l the linear expression to get the lower bound of.
     * @return utils::inf_rational the current lower bound of linear expression `l`.
     */
    inline utils::inf_rational lb(const lin &l) const noexcept
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
    inline utils::inf_rational ub(const lin &l) const noexcept
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
    inline std::pair<utils::inf_rational, utils::inf_rational> bounds(const lin &l) const noexcept
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
    inline utils::inf_rational value(const lin &l) const
    {
      utils::inf_rational val(l.known_term);
      for (const auto &[v, c] : l.vars)
        val += value(v) * c;
      return val;
    }

    /**
     * @brief Returns whether the given linear expressions can be equal.
     *
     * @param l0 the first linear expression.
     * @param l1 the second linear expression.
     * @return bool whether the given linear expressions can be equal.
     */
    SEMITONE_EXPORT bool matches(const lin &l0, const lin &l1) const noexcept;

    bool set_lb(const var &x_i, const utils::inf_rational &val, const lit &p) noexcept { return assert_lower(x_i, val, p); }
    bool set_ub(const var &x_i, const utils::inf_rational &val, const lit &p) noexcept { return assert_upper(x_i, val, p); }
    bool set(const var &x_i, const utils::inf_rational &val, const lit &p) noexcept { return set_lb(x_i, val, p) && set_ub(x_i, val, p); }

  private:
    bool propagate(const lit &p) noexcept override;
    bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

    /**
     * @brief Asserts that the lower bound of variable `x_i` is `val` and returns whether the assertion was successful.
     *
     * @param x_i the variable to assert the lower bound of.
     * @param val the lower bound to assert.
     * @param p the literal that caused the assertion.
     * @return bool whether the assertion was successful.
     */
    SEMITONE_EXPORT bool assert_lower(const var &x_i, const utils::inf_rational &val, const lit &p) noexcept;
    /**
     * @brief Asserts that the upper bound of variable `x_i` is `val` and returns whether the assertion was successful.
     *
     * @param x_i the variable to assert the upper bound of.
     * @param val the upper bound to assert.
     * @param p the literal that caused the assertion.
     * @return bool whether the assertion was successful.
     */
    SEMITONE_EXPORT bool assert_upper(const var &x_i, const utils::inf_rational &val, const lit &p) noexcept;
    void update(const var &x_i, const utils::inf_rational &v) noexcept;
    void pivot_and_update(const var &x_i, const var &x_j, const utils::inf_rational &v) noexcept;
    void pivot(const var x_i, const var x_j) noexcept;
    void new_row(const var &x, const lin &l) noexcept;

    inline void listen(const var &v, lra_value_listener *const l) noexcept
    {
      if (lb(v) < ub(v))
        listening[v].insert(l);
    }

    inline static size_t lb_index(const var &v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    inline static size_t ub_index(const var &v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    friend SEMITONE_EXPORT json::json to_json(const lra_theory &rhs) noexcept;

  private:
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value; // the value of the bound..
      lit reason;                // the reason for the value..
    };

    std::vector<bound> c_bounds;                           // the current bounds..
    std::vector<utils::inf_rational> vals;                 // the current values..
    std::map<const var, row *> tableau;                    // the sparse matrix..
    std::unordered_map<std::string, var> exprs;            // the expressions (string to numeric variable) for which already exist slack variables..
    std::unordered_map<std::string, lit> s_asrts;          // the assertions (string to literal) used for reducing the number of boolean variables..
    std::unordered_map<var, assertion *> v_asrts;          // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::vector<std::vector<assertion *>> a_watches;       // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::unordered_set<row *>> t_watches;      // for each variable `v`, a list of tableau rows watching `v`..
    std::vector<std::unordered_map<size_t, bound>> layers; // we store the updated bounds..
    std::unordered_map<var, std::set<lra_value_listener *>> listening;
  };
} // namespace semitone