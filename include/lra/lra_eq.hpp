#pragma once

#include "lin.hpp"
#include "inf_rational.hpp"
#include <optional>

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

    /**
     * @brief Notifies the equality that the lower bound of the `x_i` variable has changed.
     *
     * This function is responsible for identifying a free variable whose bound can be updated.
     *
     * @param x_i the variable whose lower bound has changed.
     * @return true if the equality is still satisfied, false otherwise.
     */
    [[nodiscard]] bool propagate_lb(const VARIABLE_TYPE x_i) noexcept;

    /**
     * @brief Notifies the equality that the upper bound of the `x_i` variable has changed.
     *
     * This function is responsible for identifying a free variable whose bound can be updated.
     *
     * @param x_i the variable whose upper bound has changed.
     * @return true if the equality is still satisfied, false otherwise.
     */
    [[nodiscard]] bool propagate_ub(const VARIABLE_TYPE x_i) noexcept;

  private:
    [[nodiscard]] std::optional<std::pair<VARIABLE_TYPE, utils::inf_rational>> free_var_lb() const noexcept;
    [[nodiscard]] std::optional<std::pair<VARIABLE_TYPE, utils::inf_rational>> free_var_ub() const noexcept;

#ifdef ENABLE_API
    friend json::json to_json(const lra_eq &rhs) noexcept;
#endif

  private:
    lra_theory &th;        // the linear real arithmetic theory..
    const VARIABLE_TYPE x; // the numeric variable..
    utils::lin l;          // the linear expression..
  };
} // namespace semitone
