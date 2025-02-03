#pragma once

#include "theory.hpp"
#include "lit.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <unordered_map>

namespace semitone
{
  class lra_assertion;
  class lra_eq;

  class lra_theory final : public theory
  {
    friend class lra_assertion;
    friend class lra_eq;

  public:
    lra_theory(network &slv) noexcept;

    utils::var add_var(std::string_view name, const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) noexcept;

    utils::lit add_constraint(bool_expr expr, bool bind = false);

  private:
    utils::lin linearize(real_expr expr);

    void new_leq(const utils::var x, const utils::inf_rational &v, bool bind = false) noexcept;
    void new_geq(const utils::var x, const utils::inf_rational &v, bool bind = false) noexcept;

  private:
    std::unordered_map<std::string, size_t> var_map;
    std::vector<utils::inf_rational> vals; // the current values..
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };
    std::vector<bound> c_bounds;                                     // the current bounds..
    std::map<const utils::var, utils::u_ptr<lra_assertion>> v_asrts; // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const utils::var, utils::u_ptr<lra_eq>> tableau;        // the tableau..
  };

  enum op
  {
    leq,
    geq
  };

  /**
   * Represents a linear real arithmetic assertion.
   *
   * This class represents a linear real arithmetic assertion of the form `b => x o v` where `b` is a boolean literal,
   * `x` is a numeric variable, `o` is an operator (either `<=` or `>=`), and `v` is a constant.
   */
  class lra_assertion
  {
    friend class lra_theory;

  public:
    lra_assertion(const utils::lit b, const utils::var x, const op o, const utils::inf_rational &v) noexcept : b(b), x(x), o(o), v(v) {}

  private:
    const utils::lit b;          // the literal associated to the assertion..
    const utils::var x;          // the numeric variable..
    const op o;                  // the operator..
    const utils::inf_rational v; // the constant..
  };

  /**
   * Represents a linear equation.
   *
   * This class represents a linear equation of the form `x = \Sum_{i=1}^{n} a_i x_i + b` where `x` is a basic numeric variable,
   * `a_i` are the coefficients, `x_i` are the numeric variables, and `b` is a constant.
   */
  class lra_eq
  {
    friend class lra_theory;

  public:
    lra_eq(const utils::var x, const utils::lin &&l) noexcept : x(x), l(std::move(l)) {}

  private:
    const utils::var x; // the basic numeric variable..
    utils::lin l;       // the linear expression..
  };
} // namespace semitone
