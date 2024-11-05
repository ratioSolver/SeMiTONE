#pragma once

#include "theory.hpp"
#include "lit.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <set>
#include <unordered_map>
#include <queue>

namespace semitone
{
  class lra_theory2;

  enum op
  {
    leq,
    geq
  };

  class lra_assertion
  {
    friend class lra_theory2;

  public:
    lra_assertion(const utils::lit b, const VARIABLE_TYPE x, const op o, const utils::inf_rational &v) noexcept : b(b), x(x), o(o), v(v) {}

  private:
    const utils::lit b;          // the literal associated to the assertion..
    const VARIABLE_TYPE x;       // the numeric variable..
    const op o;                  // the operator..
    const utils::inf_rational v; // the constant..
  };

  class lra_eq
  {
    friend class lra_theory2;

  public:
    lra_eq(const VARIABLE_TYPE x, const utils::lin &&l) noexcept : x(x), l(std::move(l)) {}

  private:
    const VARIABLE_TYPE x; // the numeric variable..
    utils::lin l;          // the linear expression..
  };

#ifdef BUILD_LISTENERS
  class lra_value_listener2;
#endif

  class lra_theory2 : public theory
  {
#ifdef BUILD_LISTENERS
    friend class lra_value_listener2;
#endif

  public:
    ~lra_theory2();

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @param lb the lower bound of the new variable.
     * @param ub the upper bound of the new variable.
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) noexcept;

    /**
     * @brief Create a new linear real arithmetic variable and make it equal to the given linear expression.
     *
     * @param l the linear expression to make the new variable equal to.
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var(const utils::lin &&l) noexcept;

    /**
     * @brief Creates a new less-than-or-equal-to literal.
     *
     * This function generates a new literal representing the inequality x <= v.
     *
     * @param x The variable to be compared.
     * @param v The value to compare the variable against, represented as an inf_rational.
     * @return A utils::lit object representing the inequality x <= v.
     */
    [[nodiscard]] utils::lit new_leq(const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept;

    /**
     * @brief Creates a new greater-than-or-equal-to literal.
     *
     * This function generates a new literal representing the inequality x >= v.
     *
     * @param x The variable to be compared.
     * @param v The inf_rational value to compare against.
     * @return A utils::lit object representing the inequality x >= v.
     */
    [[nodiscard]] utils::lit new_geq(const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept;

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

#ifdef BUILD_LISTENERS
    void add_listener(lra_value_listener2 &l) noexcept;
    void remove_listener(lra_value_listener2 &l) noexcept;
#endif

  private:
    [[nodiscard]] inline static size_t lb_index(const VARIABLE_TYPE v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    [[nodiscard]] inline static size_t ub_index(const VARIABLE_TYPE v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    [[nodiscard]] bool is_basic(const VARIABLE_TYPE v) const noexcept { return tableau.count(v); }

    [[nodiscard]] bool assert_lower(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;
    [[nodiscard]] bool assert_upper(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;

    bool propagate() noexcept;

    void update(const VARIABLE_TYPE x_i, const utils::inf_rational &v) noexcept;
    void pivot_and_update(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j, const utils::inf_rational &v) noexcept;
    void pivot(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j) noexcept;
    void new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept;

    [[nodiscard]] bool propagate(const utils::lit &) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

  private:
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };

    struct var_update
    {
      const VARIABLE_TYPE x; // the numeric variable..
      const op o;            // the operator (leq for upper bound, geq for lower bound)..
    };

    std::queue<var_update> prop_queue;                                         // propagation queue..
    std::vector<bound> c_bounds;                                               // the current bounds..
    std::vector<utils::inf_rational> vals;                                     // the current values..
    std::map<const VARIABLE_TYPE, std::unique_ptr<lra_assertion>> v_asrts;     // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const VARIABLE_TYPE, std::unique_ptr<lra_eq>> tableau;            // the tableau..
    std::vector<std::vector<std::reference_wrapper<lra_assertion>>> a_watches; // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::set<VARIABLE_TYPE>> t_watches;                            // for each variable `v`, a list of tableau rows watching `v`..
    std::unordered_map<std::string, VARIABLE_TYPE> exprs;                      // the expressions (string to numeric variable) for which already exist slack variables..
    std::unordered_map<std::string, utils::lit> s_asrts;                       // the assertions (string to literal) used for reducing the number of boolean variables..
    std::vector<std::unordered_map<size_t, bound>> layers;                     // we store the updated bounds..
#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<VARIABLE_TYPE, std::set<lra_value_listener2 *>> listening; // for each variable, the listeners listening to it..
    std::set<lra_value_listener2 *> listeners;                                    // the collection of listeners..
#endif
  };
} // namespace semitone
