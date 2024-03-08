#pragma once

#include "lin.hpp"
#include "inf_rational.hpp"

namespace semitone
{
  class lra_theory;

  class lra_eq
  {
  public:
    lra_eq(lra_theory &th, const VARIABLE_TYPE x, const utils::lin &&l) noexcept;

    bool propagate_lb(const VARIABLE_TYPE x_i) noexcept;
    bool propagate_ub(const VARIABLE_TYPE x_i) noexcept;

  private:
    utils::inf_rational lb() const noexcept;
    utils::inf_rational ub() const noexcept;

  private:
    lra_theory &th;        // the linear real arithmetic theory..
    const VARIABLE_TYPE x; // the numeric variable..
    const utils::lin l;    // the linear expression..
  };
} // namespace semitone
