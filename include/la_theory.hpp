#pragma once

#include "theory.hpp"
#include "inf_rational.hpp"
#include "lin.hpp"
#include <unordered_map>

namespace semitone
{
  class la_assertion;
  class la_eq;

  class la_theory : public theory
  {
  public:
    la_theory(network &slv) noexcept;

    void add(bool_expr expr);

    void push() noexcept override;
    void pop() noexcept override;

  private:
    [[nodiscard]] size_t add_real(std::string_view name, const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite));

    [[nodiscard]] utils::lin to_lin(int_expr expr);
    [[nodiscard]] utils::lin to_lin(real_expr expr);

  private:
    std::unordered_map<std::string, size_t> var_map;
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };

    std::vector<bound> c_bounds;                                     // the current bounds..
    std::vector<utils::inf_rational> vals;                           // the current values..
    std::map<const utils::var, utils::u_ptr<la_assertion>> v_asrts; // the assertions (literal to assertions) used for enforcing (negating) assertions..
    std::map<const utils::var, utils::u_ptr<la_eq>> tableau;        // the tableau..
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

#ifdef ENABLE_API
    friend json::json to_json(const lra_theory &rhs) noexcept;
#endif

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

#ifdef ENABLE_API
    friend json::json to_json(const lra_theory &rhs) noexcept;
#endif

  private:
    const utils::var x; // the numeric variable..
    utils::lin l;       // the linear expression..
  };
} // namespace semitone
