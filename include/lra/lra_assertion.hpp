#pragma once

#include "lit.hpp"
#include "inf_rational.hpp"

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
  public:
    lra_assertion(lra_theory &th, const op o, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept;

    bool propagate_lb(const VARIABLE_TYPE x_i) noexcept;
    bool propagate_ub(const VARIABLE_TYPE x_i) noexcept;

  private:
    lra_theory &th;              // the linear real arithmetic theory..
    const op o;                  // the kind of operator..
    const utils::lit b;          // the literal associated to the assertion..
    const VARIABLE_TYPE x;       // the numeric variable..
    const utils::inf_rational v; // the constant..
  };
} // namespace semitone
