#pragma once

#include <vector>
#include "lit.hpp"
#include "inf_rational.hpp"
#include "json.hpp"

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
    lra_assertion(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const op o, const utils::inf_rational &v) noexcept;
    virtual ~lra_assertion() = default;

    [[nodiscard]] const utils::lit &get_lit() const noexcept { return b; }
    [[nodiscard]] VARIABLE_TYPE get_var() const noexcept { return x; }
    [[nodiscard]] op get_op() const noexcept { return o; }
    [[nodiscard]] const utils::inf_rational &get_val() const noexcept { return v; }

    [[nodiscard]] virtual bool propagate_lb(const utils::inf_rational &lb) noexcept = 0;
    [[nodiscard]] virtual bool propagate_ub(const utils::inf_rational &ub) noexcept = 0;

    friend json::json to_json(const lra_assertion &rhs) noexcept;

  protected:
    lra_theory &th;              // the linear real arithmetic theory..
    const utils::lit b;          // the literal associated to the assertion..
    const VARIABLE_TYPE x;       // the numeric variable..
    const op o;                  // the operator..
    const utils::inf_rational v; // the constant..
  };

  class lra_leq : public lra_assertion
  {
  public:
    lra_leq(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : lra_assertion(th, b, x, leq, v) {}

    bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };

  class lra_geq : public lra_assertion
  {
  public:
    lra_geq(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : lra_assertion(th, b, x, geq, v) {}

    bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };
} // namespace semitone
