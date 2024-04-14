#pragma once

#include <memory>
#include <optional>
#include <queue>
#include <unordered_map>
#include "constr.hpp"
#include "theory.hpp"

#ifdef BUILD_LISTENERS
#include <set>
#endif

namespace semitone
{
  class theory;
#ifdef BUILD_LISTENERS
  class sat_value_listener;
#endif

  class sat_core
  {
    friend class constr;
    friend class theory;
#ifdef BUILD_LISTENERS
    friend class sat_value_listener;
#endif
  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    sat_core() noexcept;
    /**
     * @brief Construct a new sat core object.
     *
     * @param orig the sat core to copy.
     */
    sat_core(const sat_core &orig) noexcept;

    /**
     * @brief Create a new propositional variable
     *
     * @return The new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

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
    [[nodiscard]] bool new_clause(std::vector<utils::lit> &&lits) noexcept;

    /**
     * @brief Create a new equality constraint.
     *
     * @param left the left-hand side of the equality.
     * @param right the right-hand side of the equality.
     * @return lit the reified equality.
     */
    [[nodiscard]] utils::lit new_eq(const utils::lit &left, const utils::lit &right) noexcept;

    /**
     * @brief Create a new reified conjunction of the literals in `ls`.
     *
     * @param ls the literals of the conjunction.
     * @return lit the reified conjunction.
     * @note An empty conjunction is always `true`.
     */
    [[nodiscard]] utils::lit new_conj(std::vector<utils::lit> &&ls) noexcept;

    /**
     * @brief Create a new reified disjunction of the literals in `ls`.
     * 
     * @param ls the literals of the disjunction.
     * @return lit the reified disjunction.
     * @note An empty disjunction is always `false`.
     */
    [[nodiscard]] utils::lit new_disj(std::vector<utils::lit> &&ls) noexcept;

    /**
     * @brief Create a new reified at-most-one constraint.
     *
     * @param ls the literals of the at-most-one.
     * @return lit the reified at-most-one.
     */
    [[nodiscard]] utils::lit new_at_most_one(std::vector<utils::lit> &&ls) noexcept;

    /**
     * @brief Create a new reified exact-one constraint.
     *
     * @param ls the literals of the exact-one.
     * @return lit the reified exact-one.
     */
    [[nodiscard]] utils::lit new_exact_one(std::vector<utils::lit> &&ls) noexcept;

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    [[nodiscard]] utils::lbool value(const VARIABLE_TYPE &x) const noexcept { return assigns.at(x); }
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
     * @brief Check if the literals `p` and `q` match.
     *
     * Two literals match if they can be equal.
     *
     * @param p the first literal.
     * @param q the second literal.
     * @return bool `true` if the literals match, `false` otherwise.
     */
    [[nodiscard]] bool matches(const utils::lit &p, const utils::lit &q) const noexcept { return value(p) == value(q) || value(p) == utils::Undefined || value(q) == utils::Undefined; }

    /**
     * @brief Return the current decision level.
     *
     * @return The current decision level.
     */
    [[nodiscard]] size_t decision_level() const noexcept { return trail_lim.size(); }

    /**
     * @brief Check if the current decision level is the root level.
     *
     * @return `true` if the current decision level is the root level, `false` otherwise.
     */
    [[nodiscard]] bool root_level() const noexcept { return trail_lim.empty(); }

    /**
     * @brief Assume the literal `p` and propagate the current set of assumptions returning `false` if a conflict is detected.
     *
     * @param p the literal to assume.
     * @return bool `true` if the assumption is consistent, `false` otherwise.
     */
    [[nodiscard]] bool assume(const utils::lit &p) noexcept;

    /**
     * @brief Simplify the current set of assumptions.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    [[nodiscard]] bool simplify_db() noexcept;

    /**
     * @brief Check whether the current set of assumptions is satisfiable.
     *
     * @return bool `true` if the current set of assumptions is satisfiable, `false` otherwise.
     */
    [[nodiscard]] bool propagate() noexcept;

    /**
     * @brief Pop the last decision from the trail.
     */
    void pop() noexcept;

    /**
     * @brief Create a new theory of type `Tp` with the given arguments.
     *
     * @tparam Tp the type of the theory.
     * @tparam Args the type of the arguments.
     * @param args the arguments to pass to the theory constructor.
     * @return Tp& the new theory.
     */
    template <typename Tp, typename... Args>
    Tp &new_theory(Args &&...args)
    {
      static_assert(std::is_base_of_v<theory, Tp>, "Tp must be a subclass of theory");
      auto th = new Tp(std::forward<Args>(args)...);
      th->sat = this;
      theories.push_back(std::unique_ptr<Tp>(th));
      return *th;
    }

#ifdef BUILD_LISTENERS
    void add_listener(sat_value_listener &l) noexcept;
    void remove_listener(sat_value_listener &l) noexcept;
#endif

  private:
    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @param c The constraint that implied the literal.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    [[nodiscard]] bool enqueue(const utils::lit &p, const std::optional<std::reference_wrapper<constr>> &c = std::nullopt) noexcept;

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
    void analyze(constr &cnfl, std::vector<utils::lit> &out_learnt, size_t &out_btlevel) noexcept;

    /**
     * @brief Record the learnt clause `lits`.
     *
     * @param lits the learnt clause.
     */
    void record(std::vector<utils::lit> lits) noexcept;

  private:
    void bind(VARIABLE_TYPE v, theory &th) noexcept { binds[v].push_back(th); }

  private:
    std::vector<std::unique_ptr<constr>> constrs;                      // the collection of problem constraints..
    std::vector<std::vector<std::reference_wrapper<constr>>> watches;  // for each literal `p`, a list of constraints watching `p`..
    std::vector<utils::lbool> assigns;                                 // for each variable, the current assignment..
    std::vector<std::optional<std::reference_wrapper<constr>>> reason; // for each variable, the constraint that implied its value..
    std::vector<size_t> level;                                         // for each variable, the decision level it was assigned..

    std::queue<utils::lit> prop_queue; // propagation queue..
    std::vector<utils::lit> trail;     // the list of assignment in chronological order..
    std::vector<size_t> trail_lim;     // separator indices for different decision levels in `trail`..
    std::vector<utils::lit> decisions; // the list of decisions in chronological order..

    std::vector<std::unique_ptr<theory>> theories;                                        // all the theories..
    std::unordered_map<VARIABLE_TYPE, std::vector<std::reference_wrapper<theory>>> binds; // for each variable, the theories that depend on it..

#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<VARIABLE_TYPE, std::set<sat_value_listener *>> listening; // for each variable, the listeners listening to it..
    std::set<sat_value_listener *> listeners;                                    // the collection of listeners..
#endif
  };
} // namespace semitone
