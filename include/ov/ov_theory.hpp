#pragma once

#include <vector>
#include <unordered_map>
#include "theory.hpp"
#include "enum.hpp"

namespace semitone
{
  class ov_eq;

  class ov_theory final : public theory
  {
    friend class ov_eq;

  public:
    ov_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new variable with the given domain.
     *
     * @param domain the initial domain of the variable.
     * @param enforce_exct_one if true, the variable must take exactly one value from the domain.
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var(std::vector<std::reference_wrapper<utils::enum_val>> &&domain, const bool enforce_exct_one = true) noexcept;
    /**
     * @brief Create a new variable with the given domain.
     *
     * The presence of the values into the domain is controlled by the given literals.
     *
     * @param domain the initial domain of the variable and the literals that control the presence of the values.
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var(std::vector<std::pair<std::reference_wrapper<utils::enum_val>, utils::lit>> &&domain) noexcept;

    /**
     * @brief Create a new equality constraint.
     *
     * @param left the left-hand side of the equality.
     * @param right the right-hand side of the equality.
     * @return lit the reified equality.
     */
    [[nodiscard]] utils::lit new_eq(const VARIABLE_TYPE left, const VARIABLE_TYPE right) noexcept;

    /**
     * @brief Return the current domain of the `var` variable.
     *
     * @return The current domain of the `var` variable.
     */
    [[nodiscard]] std::vector<std::reference_wrapper<utils::enum_val>> domain(const VARIABLE_TYPE var) noexcept;

  private:
    bool propagate(const utils::lit &) noexcept override { return true; }
    bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

  private:
    std::vector<std::vector<std::pair<std::reference_wrapper<utils::enum_val>, utils::lit>>> domains;
    std::vector<bool> exact_one;
  };
} // namespace semitone
