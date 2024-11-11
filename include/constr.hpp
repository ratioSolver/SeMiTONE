#pragma once

#include <memory>
#include <vector>
#include "lit.hpp"
#include "bool.hpp"

#ifdef ENABLE_API
#include "json.hpp"
#endif

namespace semitone
{
  class sat_core;

  /**
   * This class is used for representing propositional constraints.
   */
  class constr
  {
    friend class sat_core;

  public:
    constr(sat_core &s) noexcept : sat(s) {}
    constr(const constr &) = delete;
    virtual ~constr() = default;

  private:
    constr &operator=(const constr &) = delete;

    /**
     * @brief Copy the constraint to a new solver.
     *
     * This method is used to copy the constraint to a new solver when the solver is copied.
     *
     * @param s The new solver.
     * @return The new constraint.
     */
    [[nodiscard]] virtual std::unique_ptr<constr> copy(sat_core &s) noexcept = 0;

    /**
     * @brief Propagate a literal through the constraint.
     *
     * @param p The literal to propagate.
     * @return `true` if the constraint network is consistent, `false` otherwise.
     */
    [[nodiscard]] virtual bool propagate(const utils::lit &p) noexcept = 0;
    /**
     * @brief Check if the constraint is redundant under the current assignment.
     *
     * @return `true` if the constraint is redundant, and can be removed from the network, `false` otherwise.
     */
    [[nodiscard]] virtual bool simplify() noexcept = 0;

    /**
     * @brief Get the reason for a literal.
     *
     * The reason is the set of literals that implied the literal.
     *
     * @param p The literal.
     * @return The reason for the literal.
     */
    [[nodiscard]] virtual std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept = 0;

  protected:
    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    [[nodiscard]] bool enqueue(const utils::lit &p) noexcept;
    /**
     * @brief Compute the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    [[nodiscard]] utils::lbool value(const utils::var &x) const noexcept;
    /**
     * @brief Compute the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    [[nodiscard]] utils::lbool value(const utils::lit &p) const noexcept;

    /**
     * @brief Monitors the specified literal.
     *
     * This function sets up a watch on the given literal, allowing the system
     * to track changes or events related to it.
     *
     * @param p The literal to be watched.
     */
    void watch(const utils::lit &p) noexcept;

    /**
     * @brief Stops watching the specified literal.
     *
     * This function removes the specified literal from the watch list,
     * ensuring that it is no longer monitored for changes or updates.
     *
     * @param p The literal to be unwatched.
     */
    void unwatch(const utils::lit &p) noexcept;

    /**
     * @brief Removes a constraint from the reason associated with a given variable.
     *
     * This function removes the constraint associated with the specified variable
     * from the reason list. It ensures that the constraint is no longer considered
     * in the reasoning process for the given variable.
     *
     * @param x The variable whose associated constraint is to be removed.
     */
    void remove_constr_from_reason(const utils::var &x) noexcept;

#ifdef ENABLE_API
  private:
    [[nodiscard]] virtual json::json to_json() const noexcept { return json::json(); }
    friend json::json to_json(const constr &rhs) noexcept;
#endif

  protected:
    sat_core &sat;
  };
} // namespace semitone
