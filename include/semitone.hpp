#pragma once

#include "theory.hpp"
#include "memory.hpp"
#include "bool.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#include <optional>
#include <queue>
#include <unordered_map>
#include <set>

namespace smt
{
  class clause;
  class theory;
  class la_theory;
  class dl_theory;
#ifdef BUILD_LISTENERS
  class prop_listener;
#endif

  class semitone
  {
    friend class clause;
    friend class theory;
#ifdef BUILD_LISTENERS
    friend class prop_listener;
#endif

  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    semitone() noexcept;
    virtual ~semitone() = default;

    /**
     * @brief Retrieves the linear arithmetic theory.
     *
     * This function returns a reference to the linear arithmetic theory.
     *
     * @return A reference to the linear arithmetic theory.
     */
    la_theory &get_linear_arithmetic_theory() noexcept { return la; }
    /**
     * @brief Retrieves the difference logic theory.
     *
     * This function returns a reference to the difference logic theory.
     *
     * @return A reference to the difference logic theory.
     */
    dl_theory &get_difference_logic_theory() noexcept { return dl; }

    /**
     * @brief Create a new propositional variable
     *
     * @return The new variable.
     */
    [[nodiscard]] utils::var mk_var() noexcept;

    /**
     * @brief Creates a new integer variable with optional lower and upper bounds.
     *
     * @param lb The lower bound of the integer variable. Defaults to negative infinity.
     * @param ub The upper bound of the integer variable. Defaults to positive infinity.
     * @return utils::var The newly created integer variable.
     */
    [[nodiscard]] utils::var mk_int(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) noexcept;
    /**
     * @brief Creates a new integer variable with the given linear expression.
     *
     * @param xpr The linear expression.
     * @return utils::var The newly created integer variable.
     */
    [[nodiscard]] utils::var mk_int(utils::lin &&xpr) noexcept;
    /**
     * @brief Creates a new real variable with optional lower and upper bounds.
     *
     * @param lb The lower bound of the real variable. Defaults to negative infinity.
     * @param ub The upper bound of the real variable. Defaults to positive infinity.
     * @return utils::var The newly created real variable.
     */
    [[nodiscard]] utils::var mk_real(const utils::rational &lb = utils::rational::negative_infinite, const utils::rational &ub = utils::rational::positive_infinite) noexcept;
    /**
     * @brief Creates a new real variable with the given linear expression.
     *
     * @param xpr The linear expression.
     * @return utils::var The newly created real variable.
     */
    [[nodiscard]] utils::var mk_real(utils::lin &&xpr) noexcept;

    /**
     * @brief Creates a new temporal point.
     *
     * @return utils::var The newly created temporal point.
     */
    [[nodiscard]] utils::var mk_tp() noexcept;

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
     * @brief Return the lower bound of a variable.
     *
     * @param v The variable.
     * @return The lower bound of the variable.
     */
    [[nodiscard]] utils::inf_rational arith_lb(const utils::var v) const noexcept;
    /**
     * @brief Return the upper bound of a variable.
     *
     * @param v The variable.
     * @return The upper bound of the variable.
     */
    [[nodiscard]] utils::inf_rational arith_ub(const utils::var v) const noexcept;
    /**
     * @brief Return the value of a variable.
     *
     * @param v The variable.
     * @return The value of the variable.
     */
    [[nodiscard]] utils::inf_rational arith_val(const utils::var v) const noexcept;

    /**
     * @brief Return the lower bound of a linear expression.
     *
     * @param l The linear expression.
     * @return The lower bound of the linear expression.
     */
    [[nodiscard]] utils::inf_rational arith_lb(const utils::lin &l) const noexcept;
    /**
     * @brief Return the upper bound of a linear expression.
     *
     * @param l The linear expression.
     * @return The upper bound of the linear expression.
     */
    [[nodiscard]] utils::inf_rational arith_ub(const utils::lin &l) const noexcept;
    /**
     * @brief Return the value of a linear expression.
     *
     * @param l The linear expression.
     * @return The value of the linear expression.
     */
    [[nodiscard]] utils::inf_rational arith_val(const utils::lin &l) const noexcept;

    /**
     * @brief Return the lower bound of a temporal point.
     *
     * @param v The temporal point.
     * @return The lower bound of the temporal point.
     */
    [[nodiscard]] utils::rational tp_lb(const utils::var v) const noexcept;
    /**
     * @brief Return the upper bound of a temporal point.
     *
     * @param v The temporal point.
     * @return The upper bound of the temporal point.
     */
    [[nodiscard]] utils::rational tp_ub(const utils::var v) const noexcept;
    /**
     * @brief Return the bounds of a temporal point.
     *
     * @param v The temporal point.
     * @return The bounds of the temporal point.
     */
    [[nodiscard]] std::pair<utils::rational, utils::rational> tp_bounds(const utils::var v) const noexcept;
    /**
     * @brief Return the distance between two temporal points.
     *
     * @param from The source temporal point.
     * @param to The destination temporal point.
     * @return The distance between the two temporal points.
     */
    [[nodiscard]] std::pair<utils::rational, utils::rational> tp_distance(const utils::var from, const utils::var to) const noexcept;

    /**
     * @brief Return the current decision level.
     *
     * @return The current decision level.
     */
    [[nodiscard]] size_t decision_level() const noexcept { return trail_lim.size(); }

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

    /**
     * @brief Adds a clause to the network.
     *
     * This function takes a vector of literals and adds it as a clause to the network.
     *
     * @param lits A vector of literals to be added as a clause. The vector is passed using move semantics.
     * @throw unsolvable_exception if the problem is unsolvable.
     */
    void add_clause(std::vector<utils::lit> &&lits);

    /**
     * @brief Creates a new less-than constraint between two linear expressions.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_lt(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit);
    /**
     * @brief Creates a new less-than-or-equal constraint between two linear expressions.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_le(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit);
    /**
     * @brief Creates a new equality constraint between two linear expressions.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_eq(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit)
    {
      add_le(lhs, rhs, p);
      add_le(rhs, lhs, p);
    }
    /**
     * @brief Creates a new greater-than-or-equal constraint between two linear expressions.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_ge(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit) { add_le(rhs, lhs, p); }
    /**
     * @brief Creates a new greater-than constraint between two linear expressions.
     *
     * @param lhs The left-hand side linear expression.
     * @param rhs The right-hand side linear expression.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_gt(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p = utils::TRUE_lit) { add_lt(rhs, lhs, p); }

    /**
     * @brief Creates a new difference constraint between two variables.
     *
     * @param x The first variable.
     * @param y The second variable.
     * @param d The difference between the two variables.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_distance(utils::var from, utils::var to, const utils::rational &dist, const utils::lit &p = utils::TRUE_lit);
    /**
     * @brief Creates a new difference constraint between two variables.
     *
     * @param x The first variable.
     * @param y The second variable.
     * @param min The minimum difference between the two variables.
     * @param max The maximum difference between the two variables.
     * @param p An optional literal that can be used to conditionally apply the constraint. Defaults to utils::TRUE_lit.
     */
    void add_distance(utils::var from, utils::var to, const utils::rational &min, const utils::rational &max, const utils::lit &p = utils::TRUE_lit)
    {
      add_distance(to, from, -min, p);
      add_distance(from, to, max, p);
    }

    /**
     * @brief Assume the literal `p`.
     *
     * This function assumes the literal `p` and propagates it.
     *
     * @param p The literal to assume.
     * @throw unsolvable_exception if the problem is unsolvable.
     */
    void assume(const utils::lit &p);

    /**
     * @brief Simplify the current set of assumptions.
     */
    void simplify_db();

    /**
     * @brief Check whether the current set of assumptions is satisfiable.
     *
     * This function checks whether the current set of assumptions is satisfiable.
     *
     * @throw unsolvable_exception if the problem is unsolvable.
     */
    void propagate();

    /**
     * @brief Advances to the next state.
     *
     * This function advances the state to the next state by propagating the negation of the current assumptions.
     */
    void next() noexcept;

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
    [[nodiscard]] bool enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c = std::nullopt) noexcept;

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
    void analyze(std::vector<utils::lit> &&cnfl, std::vector<utils::lit> &out_learnt, size_t &out_btlevel) noexcept;

    /**
     * @brief Record the learnt clause `lits`.
     *
     * @param lits the learnt clause.
     */
    void record(std::vector<utils::lit> &&lits) noexcept;

#ifdef BUILD_LISTENERS
    virtual void pushed() noexcept {}
    virtual void popped() noexcept {}
#endif

    friend std::ostream &operator<<(std::ostream &os, const semitone &net);

  private:
    std::vector<utils::u_ptr<theory>> theories; // all the theories..
    la_theory &la;                              // the linear arithmetic theory..
    dl_theory &dl;                              // the difference logic theory..

    std::vector<utils::lbool> assigns;                             // for each variable, the current assignment..
    std::vector<std::optional<utils::ref_wrapper<clause>>> reason; // for each variable, the clause that implied its value..
    std::vector<std::vector<utils::ref_wrapper<clause>>> watches;  // for each literal `p`, a list of clauses watching `p`..
    std::vector<utils::u_ptr<clause>> clauses;                     // the collection of problem clauses..
    std::vector<size_t> level;                                     // for each variable, the decision level it was assigned..

    std::queue<utils::lit> prop_queue; // propagation queue..
    std::vector<utils::lit> trail;     // the list of assignment in chronological order..
    std::vector<size_t> trail_lim;     // separator indices for different decision levels in `trail`..
    std::vector<utils::lit> decisions; // the list of decisions in chronological order..

    std::unordered_map<utils::var, std::set<theory *>> binds; // for each variable, the theories that depend on it..
#ifdef BUILD_LISTENERS
    std::unordered_map<utils::var, std::set<prop_listener *>> var_listeners; // for each variable, the listeners that depend on it..
#endif
  };

  /**
   * This class is used for representing propositional clauses.
   */
  class clause final
  {
    friend class semitone;

  public:
    /**
     * @brief Construct a new clause object given the `ls` literals.
     *
     * @param net the sat core.
     * @param ls the literals of the clause.
     */
    clause(semitone &net, std::vector<utils::lit> &&ls) noexcept;
    ~clause();

  private:
    [[nodiscard]] bool propagate(const utils::lit &p) noexcept;
    [[nodiscard]] bool simplify() noexcept;

    [[nodiscard]] std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept;

    friend std::ostream &operator<<(std::ostream &os, const clause &c);

  private:
    semitone &net;
    std::vector<utils::lit> lits;
  };

#ifdef BUILD_LISTENERS
  class prop_listener
  {
    friend class semitone;

  public:
    prop_listener(semitone &net) noexcept : net(net) {}
    virtual ~prop_listener()
    {
      for (const auto &v : vars)
        net.var_listeners[v].erase(this);
    }

    virtual void on_change(const utils::var &v) noexcept = 0;
    virtual void on_reset(const utils::var &v) noexcept = 0;

  protected:
    void listen(const utils::var &v) noexcept
    {
      vars.insert(v);
      net.var_listeners[v].insert(this);
    }

  private:
    semitone &net;
    std::set<utils::var> vars;
  };
#endif

  class unsolvable_exception : public std::exception
  {
  public:
    unsolvable_exception(std::vector<utils::lit> &&cnfl = {}) noexcept : cnfl(std::move(cnfl)) {}

    [[nodiscard]] const char *what() const noexcept override { return "the problem is unsolvable.."; }

  private:
    std::vector<utils::lit> cnfl;
  };

  [[nodiscard]] std::ostream &operator<<(std::ostream &os, const clause &c);
} // namespace smt
