#pragma once

#include "lin.hpp"
#include "inf_rational.hpp"

#ifdef ENABLE_API
#include "json.hpp"
#endif

namespace semitone
{
  class lra_theory;

  class lra_eq
  {
  public:
    lra_eq(lra_theory &th, const VARIABLE_TYPE x, const utils::lin &&l) noexcept;

    VARIABLE_TYPE get_var() const noexcept { return x; }
    utils::lin &get_lin() noexcept { return l; }
    const utils::lin &get_lin() const noexcept { return l; }

    [[nodiscard]] bool propagate_lb(const VARIABLE_TYPE x_i) noexcept;
    [[nodiscard]] bool propagate_ub(const VARIABLE_TYPE x_i) noexcept;

  private:
    [[nodiscard]] utils::inf_rational lb() const noexcept;
    [[nodiscard]] utils::inf_rational ub() const noexcept;

#ifdef ENABLE_API
    friend json::json to_json(const lra_eq &rhs) noexcept;
#endif

  private:
    lra_theory &th;        // the linear real arithmetic theory..
    const VARIABLE_TYPE x; // the numeric variable..
    utils::lin l;          // the linear expression..
  };
} // namespace semitone
