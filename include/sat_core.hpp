#pragma once

#include <memory>
#include <optional>
#include <queue>
#include <unordered_map>
#include "constr.hpp"
#include "theory.hpp"

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
     * @brief Construct a new sat core object.
     *
     * @param orig the sat core to copy.
     */
    sat_core(const sat_core &orig);

    /**
     * @brief Create a new propositional variable
     *
     * @return The new variable.
     */
    VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Add a new constraint to the problem.
     *
     * @param c the constraint to add.
     */
    void add_constr(std::unique_ptr<constr> c) noexcept { constrs.push_back(std::move(c)); }

    /**
     * @brief Create a new clause given the `lits` literals returning `false` if some trivial inconsistency is detected.
     *
     * @param lits the literals of the clause.
     * @return bool `true` if the clause was added, `false` otherwise.
     */
    bool new_clause(std::vector<lit> &&lits) noexcept;

    /**
     * @brief Create a new equality constraint.
     *
     * @param left the left-hand side of the equality.
     * @param right the right-hand side of the equality.
     * @return lit the reified equality.
     */
    lit new_eq(const lit &left, const lit &right) noexcept;

    /**
     * @brief Create a new reified conjunction of the literals in `ls`.
     *
     * @param ls the literals of the conjunction.
     * @return lit the reified conjunction.
     */
    lit new_conj(std::vector<lit> &&ls) noexcept;

    /**
     * @brief Create a new reified disjunction of the literals in `ls`.
     *
     * @param ls the literals of the disjunction.
     * @return lit the reified disjunction.
     */
    lit new_disj(std::vector<lit> &&ls) noexcept;

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    utils::lbool value(const VARIABLE_TYPE &x) const noexcept { return assigns.at(x); }
    /**
     * @brief Return the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    utils::lbool value(const lit &p) const noexcept
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

    /**
     * @brief Assume the literal `p` and propagate the current set of assumptions returning `false` if a conflict is detected.
     *
     * @param p the literal to assume.
     * @return bool `true` if the assumption is consistent, `false` otherwise.
     */
    bool assume(const lit &p) noexcept;

    /**
     * @brief Simplify the current set of assumptions.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    bool simplify_db() noexcept;

    /**
     * @brief Check whether the current set of assumptions is satisfiable.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    bool propagate() noexcept;

    /**
     * @brief Pop the last decision from the trail.
     */
    void pop() noexcept;

  private:
    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @param c The constraint that implied the literal.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    bool enqueue(const lit &p, const std::optional<std::reference_wrapper<constr>> &c = std::nullopt) noexcept;

    /**
     * @brief Pop the last literal from the trail.
     */
    void pop_one() noexcept;

    /**
     * @brief Analyze the conflict `cnfl` and return the learnt clause in `out_learnt` and the backtracking level in `out_btlevel`.
     *
     * @param cnfl the conflict to analyze.
     * @param out_learnt the learnt clause.
     * @param out_btlevel the backtracking level.
     */
    void analyze(constr &cnfl, std::vector<lit> &out_learnt, size_t &out_btlevel) noexcept;

    /**
     * @brief Record the learnt clause `lits`.
     *
     * @param lits the learnt clause.
     */
    void record(std::vector<lit> lits) noexcept;

  private:
    void bind(VARIABLE_TYPE v, theory &th) noexcept { binds[v].push_back(th); }

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
    std::unordered_map<VARIABLE_TYPE, std::vector<std::reference_wrapper<theory>>> binds;
  };
} // namespace semitone
