#pragma once

#include <memory>
#include <optional>
#include <queue>
#include "constr.hpp"

namespace semitone
{
  class theory;

  class sat_core
  {
    friend class constr;
    friend class theory;

  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    sat_core();

    /**
     * @brief Create a new propositional variable
     *
     * @return The new variable.
     */
    VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    inline utils::lbool value(const VARIABLE_TYPE &x) const noexcept { return assigns.at(x); }
    /**
     * @brief Return the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    inline utils::lbool value(const lit &p) const noexcept
    {
      switch (value(variable(p)))
      {
      case utils::True:
        return sign(p) ? utils::True : utils::False;
      case utils::False:
        return sign(p) ? utils::False : utils::True;
      default:
        return utils::Undefined;
      }
    }

    /**
     * @brief Return the current decision level.
     *
     * @return The current decision level.
     */
    size_t decision_level() const noexcept { return trail_lim.size(); }

    /**
     * @brief Check if the current decision level is the root level.
     *
     * @return `true` if the current decision level is the root level, `false` otherwise.
     */
    bool root_level() const noexcept { return trail_lim.empty(); }

  private:
    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @param c The constraint that implied the literal.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    bool enqueue(const lit &p, std::optional<std::reference_wrapper<constr>> c = std::nullopt) noexcept;

  private:
    std::vector<std::unique_ptr<constr>> constrs;                      // the collection of problem constraints..
    std::vector<std::vector<std::reference_wrapper<constr>>> watches;  // for each literal `p`, a list of constraints watching `p`..
    std::vector<utils::lbool> assigns;                                 // for each variable, the current assignment..
    std::vector<std::optional<std::reference_wrapper<constr>>> reason; // for each variable, the constraint that implied its value..
    std::vector<size_t> level;                                         // for each variable, the decision level it was assigned..

    std::queue<lit> prop_queue;    // propagation queue..
    std::vector<lit> trail;        // the list of assignment in chronological order..
    std::vector<size_t> trail_lim; // separator indices for different decision levels in `trail`..
    std::vector<lit> decisions;    // the list of decisions in chronological order..

    std::vector<std::unique_ptr<theory>> theories; // all the theories..
  };
} // namespace semitone
