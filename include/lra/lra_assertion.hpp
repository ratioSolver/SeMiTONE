#pragma once

#include <vector>
#include "lit.hpp"
#include "inf_rational.hpp"

#ifdef ENABLE_API
#include "json.hpp"
#endif

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

    /**
     * @brief Notifies the assertion that the lower bound of the `x` variable has changed.
     *
     * This function is responsible for updating the `b` literal if the assertion is either trivially satisfied or violated.
     *
     * @param lb the new lower bound of the `x` variable.
     * @return true if the assertion is still satisfied, false otherwise.
     */
    [[nodiscard]] virtual bool propagate_lb(const utils::inf_rational &lb) noexcept = 0;

    /**
     * @brief Notifies the assertion that the upper bound of the `x` variable has changed.
     *
     * This function is responsible for updating the `b` literal if the assertion is either trivially satisfied or violated.
     *
     * @param ub the new upper bound of the `x` variable.
     * @return true if the assertion is still satisfied, false otherwise.
     */
    [[nodiscard]] virtual bool propagate_ub(const utils::inf_rational &ub) noexcept = 0;

#ifdef ENABLE_API
    [[nodiscard]] friend json::json to_json(const lra_assertion &rhs) noexcept;
#endif

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

    [[nodiscard]] bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    [[nodiscard]] bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };

  class lra_geq : public lra_assertion
  {
  public:
    lra_geq(lra_theory &th, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : lra_assertion(th, b, x, geq, v) {}

    [[nodiscard]] bool propagate_lb(const utils::inf_rational &lb) noexcept override;
    [[nodiscard]] bool propagate_ub(const utils::inf_rational &ub) noexcept override;
  };
} // namespace semitone
