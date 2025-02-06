#pragma once

#include "theory.hpp"
#include "memory.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <set>

namespace semitone
{
  class la_assertion;
  class la_eq;

  class la_theory : public theory
  {
  public:
    la_theory(network &net) noexcept;

    [[nodiscard]] utils::var new_int(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept;
    [[nodiscard]] utils::var new_real(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept;
    [[nodiscard]] utils::var new_slack(utils::lin &&xpr) noexcept;

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

    void add_lt(utils::lin &lhs, utils::lin &rhs, bool strict = false);
    [[nodiscard]] utils::lit new_lt(utils::lin &lhs, utils::lin &rhs, bool strict = false) noexcept;
    void new_lt(utils::lit &p, utils::lin &lhs, utils::lin &rhs, bool strict = false) noexcept;

  private:
    [[nodiscard]] inline static size_t lb_index(const utils::var v) noexcept { return v << 1; }       // the index of the lower bound of the `v` variable..
    [[nodiscard]] inline static size_t ub_index(const utils::var v) noexcept { return (v << 1) ^ 1; } // the index of the upper bound of the `v` variable..

    void propagate(const utils::lit &p) noexcept override;

    void new_row(const utils::var x_i, utils::lin &&xpr) noexcept;

  private:
    std::vector<char> is_int; // the type of the variable..
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };
    std::vector<bound> c_bounds;                                          // the current bounds..
    std::vector<utils::inf_rational> vals;                                // the current values..
    std::map<const utils::var, utils::u_ptr<la_assertion>> v_asrts;       // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const utils::var, utils::u_ptr<la_eq>> tableau;              // the tableau..
    std::vector<std::vector<utils::ref_wrapper<la_assertion>>> a_watches; // for each variable `v`, a list of assertions watching `v`..
    std::vector<std::set<utils::var>> t_watches;                          // for each variable `v`, a list of tableau rows watching `v`..
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

  private:
    const utils::var x; // the numeric variable..
    utils::lin l;       // the linear expression..
  };
} // namespace semitone
