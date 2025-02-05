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

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @param lb the lower bound of the new variable.
     * @param ub the upper bound of the new variable.
     * @return utils::var the new variable.
     */
    [[nodiscard]] utils::var new_var(const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite)) noexcept;

    [[nodiscard]] utils::lit add_lt(const utils::lin &&xpr, bool bind = false);

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

    std::vector<bound> c_bounds;                                    // the current bounds..
    std::vector<utils::inf_rational> vals;                          // the current values..
    std::map<const utils::var, utils::u_ptr<la_assertion>> v_asrts; // the assertions (literal to assertions) used for enforcing the assertions..
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
