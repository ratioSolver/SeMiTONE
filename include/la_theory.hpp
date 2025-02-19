#pragma once

#include "theory.hpp"
#include "memory.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <set>
#ifdef BUILD_LISTENERS
#include <unordered_map>
#endif

namespace semitone
{
  class la_assertion;
  class la_eq;
#ifdef BUILD_LISTENERS
  class la_listener;
#endif

  class la_theory : public theory
  {
#ifdef BUILD_LISTENERS
    friend class la_listener;
#endif
  public:
    la_theory(network &net) noexcept;

    [[nodiscard]] utils::var new_int(const utils::rational &lb, const utils::rational &ub) noexcept;
    [[nodiscard]] utils::var new_int(utils::lin &&xpr) noexcept;
    [[nodiscard]] utils::var new_real(const utils::rational &lb, const utils::rational &ub) noexcept;
    [[nodiscard]] utils::var new_real(utils::lin &&xpr) noexcept;

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
     * @brief Creates a new less-than constraint between two linear expressions.
     *
     * This function creates a new less-than constraint between the two linear expressions `lhs` and `rhs` and optionally conditions it with the literal `p`.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     * @param strict A flag indicating whether the constraint is strict. Defaults to false.
     */
    void new_lt(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit, bool strict = false);

    /**
     * @brief Checks if the given variable is an integer.
     *
     * This function checks whether the specified variable is an integer by
     * looking it up in the is_int_var map.
     *
     * @param v The variable to check.
     * @return true if the variable is an integer, false otherwise.
     */
    inline bool is_int(const utils::var v) const noexcept { return is_int_var.at(v); }

  private:
    [[nodiscard]] inline static size_t lb_index(const utils::var v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    [[nodiscard]] inline static size_t ub_index(const utils::var v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

    /**
     * @brief Returns whether the variable `v` is basic.
     *
     * @param v the variable to check.
     */
    [[nodiscard]] bool is_basic(const utils::var v) const noexcept { return tableau.count(v); }

    [[nodiscard]] bool assert_lower(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;
    [[nodiscard]] bool assert_upper(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept;

    /**
     * @brief Updates the value of the non-basic variable `x_i` to `v` and adjusts the value of all the other basic variables so that all the equations remain satisfied.
     *
     * @param x_i the variable to update.
     * @param v the new value of the variable.
     */
    void update(const utils::var x_i, const utils::inf_rational &v) noexcept;

    /**
     * @brief Pivots the basic variable `x_i` with the non-basic variable `x_j` and updates the values of all the other basic variables so that all the equations remain satisfied.
     *
     * @param x_i the basic variable to pivot.
     * @param x_j the non-basic variable to pivot.
     * @param v the new value of the non-basic variable.
     */
    void pivot_and_update(const utils::var x_i, const utils::var x_j, const utils::inf_rational &v) noexcept;

    /**
     * @brief Pivots the basic variable `x_i` with the non-basic variable `x_j`.
     *
     * @param x_i the basic variable to pivot.
     * @param x_j the non-basic variable to pivot.
     */
    void pivot(const utils::var x_i, const utils::var x_j) noexcept;

    /**
     * @brief Adds a new row `x_i = xpr` to the tableau.
     *
     * This function adds a new row to the tableau with the variable `x_i` as the basic variable and the linear expression `xpr` as the right-hand side of the equation.
     *
     * @param x_i the basic variable.
     * @param xpr the linear expression.
     */
    void new_row(const utils::var x_i, utils::lin &&xpr) noexcept;

    friend std::ostream &operator<<(std::ostream &os, const la_theory &th);

  private:
    std::vector<char> is_int_var; // the type of the variable..
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };
    std::vector<bound> c_bounds;                                              // the current bounds..
    std::vector<utils::inf_rational> vals;                                    // the current values..
    std::map<const utils::var, std::set<utils::u_ptr<la_assertion>>> v_asrts; // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const utils::var, utils::u_ptr<la_eq>> tableau;                  // the tableau..
    std::vector<std::vector<utils::ref_wrapper<la_assertion>>> a_watches;     // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::set<utils::var>> t_watches;                              // for each variable `v`, a list of tableau rows watching `v`..
    std::vector<std::map<size_t, bound>> layers;                              // we store the updated bounds..
#ifdef BUILD_LISTENERS
    std::unordered_map<utils::var, std::set<la_listener *>> var_listeners; // for each variable, the listeners that depend on it..
#endif
  };

  enum op
  {
    leq,
    geq
  };

  class la_assertion
  {
    friend class la_theory;

  public:
    la_assertion(const utils::lit b, const utils::var x, const op o, const utils::inf_rational &v) noexcept : b(b), x(x), o(o), v(v) {}

    friend std::ostream &operator<<(std::ostream &os, const la_theory &th);

  private:
    const utils::lit b;          // the literal associated to the assertion..
    const utils::var x;          // the numeric variable..
    const op o;                  // the operator..
    const utils::inf_rational v; // the constant..
  };

  class la_eq
  {
    friend class la_theory;

  public:
    la_eq(const utils::var x, const utils::lin &&l) noexcept : x(x), l(std::move(l)) {}

    friend std::ostream &operator<<(std::ostream &os, const la_theory &th);

  private:
    const utils::var x; // the numeric variable..
    utils::lin l;       // the linear expression..
  };

#ifdef BUILD_LISTENERS
  class la_listener
  {
    friend class la_theory;

  public:
    la_listener(la_theory &th) noexcept : th(th) {}
    virtual ~la_listener()
    {
      for (const auto &v : vars)
        th.var_listeners[v].erase(this);
    }

    virtual void on_arith_change(const utils::var &v) noexcept = 0;

  protected:
    void listen_arith(const utils::var &v) noexcept
    {
      vars.insert(v);
      th.var_listeners[v].insert(this);
    }

  private:
    la_theory &th;
    std::set<utils::var> vars;
  };
#endif
} // namespace semitone
