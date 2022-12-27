#pragma once

#include "sat_core.h"
#include "theory.h"
#include "lin.h"
#include <limits>
#include <map>

namespace semitone
{
  class idl_value_listener;

  class idl_theory : public theory
  {
    friend class idl_value_listener;

  public:
    /**
     * @brief Construct a new integer difference logic theory object.
     *
     * @param sat the SAT solver to use.
     * @param size the initial size of the theory.
     */
    SEMITONE_EXPORT idl_theory(std::shared_ptr<sat_core> sat, const size_t &size = 16);
    SEMITONE_EXPORT idl_theory(std::shared_ptr<sat_core> sat, const idl_theory &orig);
    idl_theory(const idl_theory &orig) = delete;
    SEMITONE_EXPORT virtual ~idl_theory();

    /**
     * @brief Creates a new difference logice variable.
     *
     * @return var the new variable.
     */
    SEMITONE_EXPORT var new_var() noexcept;

    SEMITONE_EXPORT lit new_distance(const var &from, const var &to, const I &dist) noexcept; // creates and returns a new propositional literal for controlling the constraint `to - from <= dist`..
    SEMITONE_EXPORT lit new_distance(const var &from, const var &to, const I &min, const I &max) noexcept { return sat->new_conj({new_distance(to, from, -min), new_distance(from, to, max)}); }

    /**
     * @brief Creates a new lower then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_lt(const lin &left, const lin &right);
    /**
     * @brief Creates a new lower then or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_leq(const lin &left, const lin &right);
    /**
     * @brief Creates a new equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_eq(const lin &left, const lin &right);
    /**
     * @brief Creates a new greater then or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_geq(const lin &left, const lin &right);
    /**
     * @brief Creates a new greater then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return lit the literal corresponding to the constraint.
     */
    SEMITONE_EXPORT lit new_gt(const lin &left, const lin &right);

    /**
     * @brief Returns the lower bound of the given variable.
     *
     * @param v the variable to get the lower bound of.
     * @return I the lower bound of the variable.
     */
    inline I lb(const var &v) const noexcept { return -_dists[v][0]; }
    /**
     * @brief Returns the upper bound of the given variable.
     *
     * @param v the variable to get the upper bound of.
     * @return I the upper bound of the variable.
     */
    inline I ub(const var &v) const noexcept { return _dists[0][v]; }
    /**
     * @brief Returns the bounds of the given variable.
     *
     * @param v the variable to get the bounds of.
     * @return std::pair<I, I> the bounds of the variable.
     */
    inline std::pair<I, I> bounds(const var &v) const noexcept { return std::make_pair(-_dists[v][0], _dists[0][v]); }
    /**
     * @brief Returns the distance between the given variables.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @return std::pair<I, I> the distance between the variables.
     */
    inline std::pair<I, I> distance(const var &from, const var &to) const noexcept { return std::make_pair(-_dists[to][from], _dists[from][to]); }

    /**
     * @brief Returns the bounds of the given linear expression.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<I, I> the bounds of the linear expression.
     */
    SEMITONE_EXPORT std::pair<I, I> bounds(const lin &l) const;
    /**
     * @brief Returns the distance between the given linear expressions.
     *
     * @param from the linear expression to get the distance from.
     * @param to the linear expression to get the distance to.
     * @return std::pair<I, I> the distance between the linear expressions.
     */
    SEMITONE_EXPORT std::pair<I, I> distance(const lin &from, const lin &to) const;

    /**
     * @brief Returns whether the given linear expressions can be equal.
     *
     * @param l0 the first linear expression.
     * @param l1 the second linear expression.
     * @return bool whether the linear expressions can be equal.
     */
    SEMITONE_EXPORT bool matches(const lin &l0, const lin &l1) const;

    /**
     * @brief Returns the number of variables in the theory.
     *
     * @return size_t the number of variables in the theory.
     */
    size_t size() const noexcept { return n_vars; }

  public:
    inline static constexpr I inf() noexcept { return std::numeric_limits<I>::max() / 2 - 1; }

  private:
    bool propagate(const lit &p) noexcept override;
    bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

    void propagate(const var &from, const var &to, const I &dist) noexcept;
    void set_dist(const var &from, const var &to, const I &dist) noexcept;
    void set_pred(const var &from, const var &to, const var &pred) noexcept;

    void resize(const size_t &size) noexcept;

    inline void listen(const var &v, idl_value_listener *const l) noexcept { listening[v].insert(l); }

  private:
    class idl_distance final
    {
      friend class idl_theory;

    public:
      idl_distance(const lit &b, const var &from, const var &to, const I &dist) : b(b), from(from), to(to), dist(dist) {}
      idl_distance(const idl_distance &orig) = delete;

    private:
      const lit b; // the propositional literal associated to the distance constraint..
      const var from;
      const var to;
      const I dist;
    };

    struct layer
    {
      std::map<std::pair<var, var>, I> old_dists;                // the updated distances..
      std::map<std::pair<var, var>, var> old_preds;              // the updated predecessors..
      std::map<std::pair<var, var>, idl_distance *> old_constrs; // the updated constraints..
    };

    size_t n_vars = 1;
    std::vector<std::vector<I>> _dists;                                      // the distance matrix..
    std::vector<std::vector<var>> _preds;                                    // the predecessor matrix..
    std::map<std::pair<var, var>, idl_distance *> dist_constr;               // the currently enforced constraints..
    std::unordered_map<var, idl_distance *> var_dists;                       // the constraints controlled by a propositional variable (for propagation purposes)..
    std::map<std::pair<var, var>, std::vector<idl_distance *>> dist_constrs; // the constraints between two temporal points (for propagation purposes)..
    std::vector<layer> layers;                                               // we store the updates..
    std::unordered_map<var, std::set<idl_value_listener *>> listening;
  };
} // namespace semitone