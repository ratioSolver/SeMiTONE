#pragma once

#include "theory.hpp"
#include "lit.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <set>
#include <unordered_map>
#ifdef ENABLE_API
#include "json.hpp"
#endif

namespace semitone
{
  class lra_theory;

  enum op
  {
    leq,
    geq
  };

  class lra_assertion
  {
    friend class lra_theory;

  public:
    lra_assertion(const utils::lit b, const utils::var x, const op o, const utils::inf_rational &v) noexcept : b(b), x(x), o(o), v(v) {}

#ifdef ENABLE_API
    friend json::json to_json(const lra_theory &rhs) noexcept;
#endif

  private:
    const utils::lit b;          // the literal associated to the assertion..
    const utils::var x;          // the numeric variable..
    const op o;                  // the operator..
    const utils::inf_rational v; // the constant..
  };

  class lra_eq
  {
    friend class lra_theory;

  public:
    lra_eq(const utils::var x, const utils::lin &&l) noexcept : x(x), l(std::move(l)) {}

#ifdef ENABLE_API
    friend json::json to_json(const lra_theory &rhs) noexcept;
#endif

  private:
    const utils::var x; // the numeric variable..
    utils::lin l;       // the linear expression..
  };

#ifdef BUILD_LISTENERS
  class lra_value_listener;
#endif

  class lra_theory : public theory
  {
#ifdef BUILD_LISTENERS
    friend class lra_value_listener;
#endif

  public:
    ~lra_theory();

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @param lb the lower bound of the new variable.
     * @param ub the upper bound of the new variable.
     * @return utils::var the new variable.
     */
    [[nodiscard]] utils::var new_var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) noexcept;

    /**
     * @brief Create a new linear real arithmetic variable and make it equal to the given linear expression.
     *
     * @param l the linear expression to make the new variable equal to.
     * @return utils::var the new variable.
     */
    [[nodiscard]] utils::var new_var(const utils::lin &&l) noexcept;

    /**
     * @brief Creates a new less-than-or-equal-to literal.
     *
     * This function generates a new literal representing the inequality x <= v.
     *
     * @param x The variable to be compared.
     * @param v The value to compare the variable against, represented as an inf_rational.
     * @return A utils::lit object representing the inequality x <= v.
     */
    [[nodiscard]] utils::lit new_leq(const utils::var x, const utils::inf_rational &v) noexcept;

    /**
     * @brief Creates a new greater-than-or-equal-to literal.
     *
     * This function generates a new literal representing the inequality x >= v.
     *
     * @param x The variable to be compared.
     * @param v The inf_rational value to compare against.
     * @return A utils::lit object representing the inequality x >= v.
     */
    [[nodiscard]] utils::lit new_geq(const utils::var x, const utils::inf_rational &v) noexcept;

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
     * @brief Returns the current lower bound of variable `v`.
     *
     * @param v the variable to get the lower bound of.
     * @return utils::inf_rational the current lower bound of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational lb(const utils::var v) const noexcept { return c_bounds[lb_index(v)].value; }
    /**
     * @brief Returns the current upper bound of variable `v`.
     *
     * @param v the variable to get the upper bound of.
     * @return utils::inf_rational the current upper bound of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational ub(const utils::var v) const noexcept { return c_bounds[ub_index(v)].value; }
    /**
     * @brief Returns the current value of variable `v`.
     *
     * @param v the variable to get the value of.
     * @return utils::inf_rational the current value of variable `v`.
     */
    [[nodiscard]] inline utils::inf_rational value(const utils::var v) const noexcept { return vals[v]; }

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
      return {c_lb, c_ub};
    }

    /**
     * @brief Returns whether the two linear expressions overlap. That is, whether the two linear expressions can have a common value.
     *
     * @param l0 the first linear expression.
     * @param l1 the second linear expression.
     * @return bool whether the two linear expressions overlap.
     */
    [[nodiscard]] bool matches(const utils::lin &l0, const utils::lin &l1) const noexcept
    {
      const auto [l0_lb, l0_ub] = bounds(l0);
      const auto [l1_lb, l1_ub] = bounds(l1);
      return l0_ub >= l1_lb && l0_lb <= l1_ub; // the two intervals overlap..
    }

    /**
     * @brief Sets the lower bound of variable `x_i` to `val` and propagates the change, returning whether the propagation was successful.
     *
     * @param x_i the variable to set the lower bound of.
     * @param val the lower bound to set.
     * @param r the literals that caused the change.
     * @return bool whether the propagation was successful.
     */
    [[nodiscard]] bool set_lb(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r = {}) noexcept;
    /**
     * @brief Sets the upper bound of variable `x_i` to `val` and propagates the change, returning whether the propagation was successful.
     *
     * @param x_i the variable to set the upper bound of.
     * @param val the upper bound to set.
     * @param r the literals that caused the change.
     * @return bool whether the propagation was successful.
     */
    [[nodiscard]] bool set_ub(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r = {}) noexcept;

#ifdef BUILD_LISTENERS
    void add_listener(lra_value_listener &l) noexcept;
    void remove_listener(lra_value_listener &l) noexcept;
#endif

  private:
    [[nodiscard]] std::pair<utils::inf_rational, std::vector<utils::lit>> lb_and_reason(const utils::lin &l) const noexcept;
    [[nodiscard]] std::pair<utils::inf_rational, std::vector<utils::lit>> ub_and_reason(const utils::lin &l) const noexcept;

    [[nodiscard]] inline static size_t lb_index(const utils::var v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    [[nodiscard]] inline static size_t ub_index(const utils::var v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    [[nodiscard]] bool is_basic(const utils::var v) const noexcept { return tableau.count(v); }

    [[nodiscard]] bool assert_lower(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;
    [[nodiscard]] bool assert_upper(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;

    void update(const utils::var x_i, const utils::inf_rational &v) noexcept;
    void pivot_and_update(const utils::var x_i, const utils::var x_j, const utils::inf_rational &v) noexcept;
    void pivot(const utils::var x_i, const utils::var x_j) noexcept;
    void new_row(const utils::var x_i, const utils::lin &&xpr) noexcept;

    [[nodiscard]] bool propagate(const utils::lit &) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

#ifdef ENABLE_API
    friend json::json to_json(const lra_theory &rhs) noexcept;
#endif

  private:
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };

    std::vector<bound> c_bounds;                                               // the current bounds..
    std::vector<utils::inf_rational> vals;                                     // the current values..
    std::map<const utils::var, std::unique_ptr<lra_assertion>> v_asrts;        // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const utils::var, std::unique_ptr<lra_eq>> tableau;               // the tableau..
    std::vector<std::vector<std::reference_wrapper<lra_assertion>>> a_watches; // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::set<utils::var>> t_watches;                               // for each variable `v`, a list of tableau rows watching `v`..
    std::unordered_map<std::string, utils::var> exprs;                         // the expressions (string to numeric variable) for which already exist slack variables..
    std::unordered_map<std::string, utils::lit> s_asrts;                       // the assertions (string to literal) used for reducing the number of boolean variables..
    std::vector<std::unordered_map<size_t, bound>> layers;                     // we store the updated bounds..
#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<utils::var, std::set<lra_value_listener *>> listening; // for each variable, the listeners listening to it..
    std::set<lra_value_listener *> listeners;                                 // the collection of listeners..
#endif
  };
} // namespace semitone
