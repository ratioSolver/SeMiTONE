#pragma once

#include "context.hpp"
#include "lit.hpp"
#include <optional>
#include <unordered_map>
#include <queue>

namespace semitone
{
  class clause;

  class solver
  {
  public:
    solver(context &ctx);

    void add(bool_expr expr);

  private:
    [[nodiscard]] bool_expr to_cnf(bool_expr expr);
    [[nodiscard]] bool_expr push_negations(bool_expr expr);
    [[nodiscard]] bool_expr distribute(bool_expr expr);

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
  };

  class clause
  {
    friend class solver;
    /**
     * @brief Construct a new clause object given the `lits` literals.
     *
     * @param slv The solver.
     * @param lits The literals of the clause.
     */
    clause(solver &slv, std::vector<utils::lit> &&lits) noexcept;

  private:
    solver &slv;
    std::vector<utils::lit> lits;
  };

  class unsolvable_exception : public std::exception
  {
    const char *what() const noexcept override { return "the problem is unsolvable.."; }
  };
} // namespace semitone
