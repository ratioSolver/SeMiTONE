#pragma once

#include "context.hpp"
#include "lit.hpp"
#include "theory.hpp"
#include <optional>
#include <unordered_map>
#include <queue>

namespace semitone
{
  class clause;
  class la_theory;

  class network
  {
    friend class clause;

  public:
    network(context &ctx);

    /**
     * @brief Adds a boolean expression to the solver.
     *
     * This function takes a boolean expression and adds it to the solver's internal
     * data structures. The expression will be used in subsequent solving operations.
     *
     * @param expr The boolean expression to be added.
     */
    void add(bool_expr expr);

    /**
     * @brief Propagates constraints in the solver.
     *
     * This function attempts to propagate constraints within the solver.
     * It ensures that all constraints are satisfied and updates the internal
     * state accordingly.
     *
     * @return true if propagation was successful and all constraints are satisfied.
     * @return false if propagation failed or if any constraint is violated.
     */
    bool propagate() noexcept;

    /**
     * @brief Assume a boolean expression.
     *
     * This function assumes a boolean expression to be true. It adds the expression
     * to the solver's internal data structures and propagates the constraints.
     *
     * @param expr The boolean expression to assume.
     * @return `true` if the assumption is consistent, `false` otherwise.
     */
    bool assume(bool_expr expr) noexcept;

    /**
     * @brief Pop the last decision from the trail.
     */
    void pop() noexcept;

    /**
     * @brief Evaluates the given expression.
     *
     * This function updates the value of the given expression based on the current
     * assignment of the variables.
     *
     * @param xpr The expression to be evaluated.
     */
    utils::lbool eval(bool_expr xpr);

    context &get_context() noexcept { return ctx; }

  private:
    [[nodiscard]] size_t add_var(std::string_view name);

    void add_clause(bool_expr expr);

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    [[nodiscard]] inline utils::lbool value(const utils::var &x) const noexcept { return assigns.at(x); }
    /**
     * @brief Return the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    [[nodiscard]] inline utils::lbool value(const utils::lit &p) const noexcept
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

    /**
     * @brief Enqueue a literal in the assignment.
     *
     * @param p The literal to enqueue.
     * @param c The clause that implied the literal.
     * @return `true` if the assignment is consistent, `false` otherwise.
     */
    [[nodiscard]] bool enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c = std::nullopt) noexcept;

    /**
     * @brief Analyze the conflict `cnfl` and return the learnt clause in `out_learnt` and the backtracking level in `out_btlevel`.
     *
     * @param cnfl the conflict to analyze.
     * @param out_learnt the learnt clause.
     * @param out_btlevel the backtracking level.
     */
    void analyze(clause &cnfl, std::vector<utils::lit> &out_learnt, size_t &out_btlevel) noexcept;

    /**
     * @brief Record the learnt clause `lits`.
     *
     * @param lits the learnt clause.
     */
    void record(std::vector<utils::lit> lits) noexcept;

    /**
     * @brief Pop the last literal from the trail.
     */
    void pop_one() noexcept;

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
      theories.push_back(th);
      return *th;
    }

  private:
    context &ctx;
    std::unordered_map<std::string, size_t> var_map;
    std::vector<std::vector<utils::ref_wrapper<clause>>> watches;  // for each literal `p`, a list of clauses watching `p`..
    std::vector<utils::lbool> assigns;                             // the current assignments..
    std::vector<std::optional<utils::ref_wrapper<clause>>> reason; // for each variable, the clause that implied its value..
    std::vector<utils::u_ptr<clause>> clauses;                     // the collection of problem clauses..
    std::vector<size_t> level;                                     // for each variable, the decision level it was assigned..

    std::queue<utils::lit> prop_queue; // propagation queue..
    std::vector<utils::lit> trail;     // the list of assignment in chronological order..
    std::vector<size_t> trail_lim;     // separator indices for different decision levels in `trail`..

    std::vector<utils::u_ptr<theory>> theories; // all the theories..
    la_theory &la;                              // the linear arithmetic theory..
  };

  class clause
  {
    friend class network;
    /**
     * @brief Construct a new clause object given the `lits` literals.
     *
     * @param slv The solver.
     * @param lits The literals of the clause.
     */
    clause(network &slv, std::vector<utils::lit> &&lits) noexcept;

    [[nodiscard]] bool propagate(const utils::lit &p) noexcept;
    [[nodiscard]] bool simplify() noexcept;

    [[nodiscard]] std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept;

  private:
    network &slv;
    std::vector<utils::lit> lits;
  };

  class unsolvable_exception : public std::exception
  {
    const char *what() const noexcept override { return "the problem is unsolvable.."; }
  };
} // namespace semitone
