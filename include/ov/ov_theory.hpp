#pragma once

#include <vector>
#include <unordered_map>
#include "theory.hpp"
#include "enum.hpp"
#ifdef BUILD_LISTENERS
#include <set>
#endif

namespace semitone
{
  class ov_eq;
#ifdef BUILD_LISTENERS
  class ov_value_listener;
#endif

  class ov_theory final : public theory
  {
    friend class ov_eq;
#ifdef BUILD_LISTENERS
    friend class ov_value_listener;
#endif

  public:
    ~ov_theory();

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
    [[nodiscard]] std::vector<std::reference_wrapper<utils::enum_val>> domain(const VARIABLE_TYPE var) const noexcept;

    /**
     * @brief Check if the given value is allowed to the variable.
     *
     * @param var the variable to check.
     * @param val the value to check.
     * @return lit the literal that represents the presence of the value in the domain.
     */
    [[nodiscard]] utils::lit allows(const VARIABLE_TYPE var, utils::enum_val &val) const noexcept;

    /**
     * @brief Assign the given value to the variable.
     *
     * @param var the variable to assign.
     * @param val the value to assign.
     * @return true if the assignment is successful, false otherwise.
     */
    [[nodiscard]] bool assign(const VARIABLE_TYPE var, utils::enum_val &val) noexcept;

    /**
     * @brief Forbid the given value to the variable.
     *
     * @param var the variable to forbid.
     * @param val the value to forbid.
     * @return true if the forbidding is successful, false otherwise.
     */
    [[nodiscard]] bool forbid(const VARIABLE_TYPE var, utils::enum_val &val) noexcept;

    /**
     * @brief Checks if the given variables match.
     *
     * Two variables match if they can be equal.
     *
     * @param v0 the first variable.
     * @param v1 the second variable.
     * @return true if the variables match.
     * @return false otherwise.
     */
    [[nodiscard]] bool matches(const VARIABLE_TYPE v0, const VARIABLE_TYPE v1);

#ifdef BUILD_LISTENERS
    void add_listener(ov_value_listener &l) noexcept;
    void remove_listener(ov_value_listener &l) noexcept;
#endif

  private:
    [[nodiscard]] bool propagate(const utils::lit &) noexcept override { return true; }
    [[nodiscard]] bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

  private:
    std::vector<std::unordered_map<utils::enum_val *, utils::lit>> domains;
#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<VARIABLE_TYPE, std::set<ov_value_listener *>> listening; // for each variable, the listeners listening to it..
    std::set<ov_value_listener *> listeners;                                    // the collection of listeners..
#endif
  };
} // namespace semitone
