#pragma once

#include "lin.hpp"

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
    lra_theory &th;        // the linear real arithmetic theory..
    const VARIABLE_TYPE x; // the numeric variable..
    const utils::lin l;    // the linear expression..
  };
} // namespace semitone
