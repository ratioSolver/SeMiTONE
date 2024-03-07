#pragma once

#include "lit.hpp"
#include "inf_rational.hpp"

namespace semitone
{
  class lra_theory;

  class lra_assertion
  {
  public:
    lra_assertion(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : th(th), b(b), x(x), v(v) {}

    virtual bool propagate_lb(const utils::inf_rational &lb) noexcept = 0;
    virtual bool propagate_ub(const utils::inf_rational &ub) noexcept = 0;

  protected:
    lra_theory &th;              // the linear real arithmetic theory..
    const utils::lit b;          // the literal associated to the assertion..
    const VARIABLE_TYPE x;       // the numeric variable..
    const utils::inf_rational v; // the constant..
  };

  class lra_leq : public lra_assertion
  {
  public:
    lra_leq(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : lra_assertion(th, b, x, v) {}

    bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };

  class lra_geq : public lra_assertion
  {
  public:
    lra_geq(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : lra_assertion(th, b, x, v) {}

    bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };
} // namespace semitone
