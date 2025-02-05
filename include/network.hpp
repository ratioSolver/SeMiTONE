#pragma once

#include "theory.hpp"
#include "memory.hpp"
#include "bool.hpp"
#include <optional>
#include <queue>
#include <unordered_map>
#include <set>

namespace semitone
{
  class clause;

  class network
  {
    friend class clause;

  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    network() noexcept;

    /**
     * @brief Create a new propositional variable
     *
     * @return The new variable.
     */
    [[nodiscard]] utils::var new_var() noexcept;

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    [[nodiscard]] utils::lbool value(const utils::var &x) const noexcept { return assigns.at(x); }
    /**
     * @brief Return the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    [[nodiscard]] utils::lbool value(const utils::lit &p) const noexcept
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
    [[nodiscard]] size_t decision_level() const noexcept { return trail_lim.size(); }

  private:
    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @param c The constraint that implied the literal.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    [[nodiscard]] bool enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c = std::nullopt) noexcept;

  private:
    std::vector<utils::u_ptr<clause>> clauses;                     // the collection of problem clauses..
    std::vector<std::vector<utils::ref_wrapper<clause>>> watches;  // for each literal `p`, a list of clauses watching `p`..
    std::vector<utils::lbool> assigns;                             // for each variable, the current assignment..
    std::vector<std::optional<utils::ref_wrapper<clause>>> reason; // for each variable, the clause that implied its value..
    std::vector<size_t> level;                                     // for each variable, the decision level it was assigned..

    std::queue<utils::lit> prop_queue; // propagation queue..
    std::vector<utils::lit> trail;     // the list of assignment in chronological order..
    std::vector<size_t> trail_lim;     // separator indices for different decision levels in `trail`..
    std::vector<utils::lit> decisions; // the list of decisions in chronological order..

    std::vector<utils::u_ptr<theory>> theories;                                 // all the theories..
    std::unordered_map<utils::var, std::set<utils::ref_wrapper<theory>>> binds; // for each variable, the theories that depend on it..
  };

  /**
   * This class is used for representing propositional clauses.
   */
  class clause final
  {
  public:
    /**
     * @brief Construct a new clause object given the `lits` literals.
     *
     * @param net the sat core.
     * @param lits the literals of the clause.
     */
    clause(network &net, std::vector<utils::lit> &&ls) noexcept;

  private:
    [[nodiscard]] bool propagate(const utils::lit &p) noexcept;
    [[nodiscard]] bool simplify() noexcept;

    [[nodiscard]] std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept;

  private:
    network &net;
    std::vector<utils::lit> lits;
  };
} // namespace semitone
