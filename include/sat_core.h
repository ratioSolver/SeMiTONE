#pragma once

#include "semitone_export.h"
#include "constr.h"
#include "memory.h"
#include "logging.h"
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <set>

namespace semitone
{
  class sat_stack;
  using constr_ptr = utils::u_ptr<constr>;
  class theory;
  class sat_value_listener;

  class sat_core final : public utils::countable
  {
    friend class sat_stack;
    friend class constr;
    friend class theory;
    friend class sat_value_listener;

  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    SEMITONE_EXPORT sat_core();
    /**
     * @brief Construct a new sat core object.
     *
     * @param orig the sat core to copy.
     */
    SEMITONE_EXPORT sat_core(const sat_core &orig);
    SEMITONE_EXPORT sat_core(sat_core &&orig) = default;
    SEMITONE_EXPORT ~sat_core();

    /**
     * @brief Create a new propositional variable
     *
     * @return var the new variable.
     */
    SEMITONE_EXPORT var new_var() noexcept;

    /**
     * @brief Create a new clause given the `lits` literals returning `false` if some trivial inconsistency is detected.
     *
     * @param lits the literals of the clause.
     * @return bool `true` if the clause was added, `false` otherwise.
     */
    SEMITONE_EXPORT bool new_clause(std::vector<lit> lits) noexcept;

    /**
     * @brief Create a new reified equality between `left` and `right`.
     *
     * @param left the left-hand side of the equality.
     * @param right the right-hand side of the equality.
     * @return lit the reified equality.
     */
    SEMITONE_EXPORT lit new_eq(const lit &left, const lit &right) noexcept;
    /**
     * @brief Create a new reified conjunction of the literals in `ls`.
     *
     * @param ls the literals of the conjunction.
     * @return lit the reified conjunction.
     */
    SEMITONE_EXPORT lit new_conj(std::vector<lit> ls) noexcept;
    /**
     * @brief Create a new reified disjunction of the literals in `ls`.
     *
     * @param ls the literals of the disjunction.
     * @return lit the reified disjunction.
     */
    SEMITONE_EXPORT lit new_disj(std::vector<lit> ls) noexcept;
    /**
     * @brief Create a new reified at-most-one of the literals in `ls`.
     *
     * @param ls the literals of the at-most-one.
     * @return lit the reified at-most-one.
     */
    SEMITONE_EXPORT lit new_at_most_one(std::vector<lit> ls) noexcept;
    /**
     * @brief Create a new reified exactly-one of the literals in `ls`.
     *
     * @param ls the literals of the exactly-one.
     * @return lit the reified exactly-one.
     */
    SEMITONE_EXPORT lit new_exct_one(std::vector<lit> ls) noexcept;

    /**
     * @brief Assume the literal `p`.
     *
     * @param p the literal to assume.
     * @return bool `true` if the assumption is consistent, `false` otherwise.
     */
    SEMITONE_EXPORT bool assume(const lit &p) noexcept;
    /**
     * @brief Pop the last assumption.
     *
     */
    SEMITONE_EXPORT void pop() noexcept;
    /**
     * @brief Simplify the current set of assumptions.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    SEMITONE_EXPORT bool simplify_db() noexcept;
    /**
     * @brief Check whether the current set of assumptions is satisfiable.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    SEMITONE_EXPORT bool propagate() noexcept;
    SEMITONE_EXPORT bool next() noexcept;
    SEMITONE_EXPORT bool check(std::vector<lit> lits) noexcept;

    inline utils::lbool value(const var &x) const noexcept { return assigns.at(x); } // returns the value of variable `x`..
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
    }                                                                                            // returns the value of literal `p`..
    inline size_t decision_level() const noexcept { return trail_lim.size(); }                   // returns the current decision level..
    inline bool root_level() const noexcept { return trail_lim.empty(); }                        // checks whether the current decision level is root level..
    SEMITONE_EXPORT const std::vector<lit> &get_decisions() const noexcept { return decisions; } // returns the decisions taken so far in chronological order..

  private:
    /**
     * @brief Analyze the conflict `cnfl` and return the learnt clause in `out_learnt` and the backtracking level in `out_btlevel`.
     *
     * @param cnfl the conflict to analyze.
     * @param out_learnt the learnt clause.
     * @param out_btlevel the backtracking level.
     */
    void analyze(constr &cnfl, std::vector<lit> &out_learnt, size_t &out_btlevel) noexcept;
    void record(std::vector<lit> lits) noexcept;

    bool enqueue(const lit &p, constr *const c = nullptr) noexcept;
    void pop_one() noexcept;

    inline void bind(const var &v, theory &th) noexcept { bounds[v].insert(&th); }
    inline void listen(const var &v, sat_value_listener &l) noexcept
    {
      if (value(v) == utils::Undefined)
        listening[v].insert(&l);
    }

    friend SEMITONE_EXPORT json::json to_json(const sat_core &rhs) noexcept;

  private:
    std::vector<constr_ptr> constrs;            // the collection of problem constraints..
    std::vector<std::vector<constr *>> watches; // for each literal `p`, a list of constraints watching `p`..
    std::vector<utils::lbool> assigns;          // the current assignments..

    std::queue<lit> prop_q;                     // propagation queue..
    std::vector<lit> trail;                     // the list of assignment in chronological order..
    std::vector<size_t> trail_lim;              // separator indices for different decision levels in `trail`..
    std::vector<lit> decisions;                 // the list of decisions in chronological order..
    std::vector<constr *> reason;               // for each variable, the constraint that implied its value..
    std::vector<size_t> level;                  // for each variable, the decision level it was assigned..
    std::unordered_map<std::string, lit> exprs; // the already existing expressions (string to literal)..

    std::vector<theory *> theories; // all the theories..
    std::unordered_map<size_t, std::set<theory *>> bounds;
    std::vector<sat_value_listener *> listeners; // all the listeners..
    std::unordered_map<size_t, std::set<sat_value_listener *>> listening;
  };

} // namespace semitone