#pragma once

#include <vector>
#include <unordered_set>
#include "theory.hpp"
#include "enum.hpp"

namespace semitone
{
  class ov_theory final : public theory
  {
  public:
    ov_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new variable with the given domain.
     *
     * @param domain the initial domain of the variable.
     */
    VARIABLE_TYPE new_var(std::vector<std::reference_wrapper<utils::enum_val>> &&domain, const bool enforce_exct_one = true) noexcept;
    /**
     * @brief Create a new variable with the given domain.
     *
     * The presence of the values into the domain is controlled by the given literals.
     */
    VARIABLE_TYPE new_var(std::vector<std::pair<lit, std::reference_wrapper<utils::enum_val>>> &&domain) noexcept;

    /**
     * @brief Create a new equality constraint.
     *
     * @param left the left-hand side of the equality.
     * @param right the right-hand side of the equality.
     * @return lit the reified equality.
     */
    lit new_eq(const VARIABLE_TYPE left, const VARIABLE_TYPE right) noexcept;

    /**
     * @brief Return the current domain of the `var` variable.
     *
     * @return The current domain of the `var` variable.
     */
    std::unordered_set<std::reference_wrapper<utils::enum_val>> domain(const VARIABLE_TYPE var) noexcept;

  private:
    bool propagate(const lit &p) noexcept override;

    bool check() noexcept override;

    void push() noexcept override;

    void pop() noexcept override;
  };
} // namespace semitone
