#pragma once

#include <vector>
#include "theory.hpp"
#include "dl_distance_constraint.hpp"
#include "lin.hpp"
#ifdef BUILD_LISTENERS
#include <set>
#endif

namespace semitone
{
#ifdef BUILD_LISTENERS
  class idl_value_listener;
#endif

  class idl_theory final : public theory
  {
#ifdef BUILD_LISTENERS
    friend class idl_value_listener;
#endif

  public:
    idl_theory(const size_t &size = 16) noexcept;
    idl_theory(const idl_theory &orig) noexcept;

    /**
     * @brief Create a new difference logic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Creates a new distance between the given variables and returns the corresponding literal.
     *
     * The enforced constraint is `to - from <= dist`.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @param dist the distance between the variables.
     * @return utils::lit the literal corresponding to the distance.
     */
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INT_TYPE dist) noexcept;
    /**
     * @brief Creates a new distance between the given variables and returns the corresponding literal.
     *
     * The enforced constraint is `to - from >= min && to - from <= max`.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @param min the minimum distance between the variables.
     * @param max the maximum distance between the variables.
     * @return utils::lit the literal corresponding to the distance.
     */
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INT_TYPE min, INT_TYPE max) noexcept;

    /**
     * @brief Creates a new lower then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return utils::lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_lt(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new lower then or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return utils::lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_leq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return utils::lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_eq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new greater then or equal constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return utils::lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_geq(const utils::lin &left, const utils::lin &right) noexcept;
    /**
     * @brief Creates a new greater then constraint between the given linear expressions and returns the corresponding literal.
     *
     * @param left the left hand side of the constraint.
     * @param right the right hand side of the constraint.
     * @return utils::lit the literal corresponding to the constraint.
     */
    [[nodiscard]] utils::lit new_gt(const utils::lin &left, const utils::lin &right) noexcept;

    /**
     * @brief Returns the lower bound of the given variable.
     *
     * @param v the variable to get the lower bound of.
     * @return VARIABLE_TYPE the lower bound of the variable.
     */
    [[nodiscard]] inline VARIABLE_TYPE lb(VARIABLE_TYPE v) const noexcept { return -dists[v][0]; }
    /**
     * @brief Returns the upper bound of the given variable.
     *
     * @param v the variable to get the upper bound of.
     * @return VARIABLE_TYPE the upper bound of the variable.
     */
    [[nodiscard]] inline VARIABLE_TYPE ub(VARIABLE_TYPE v) const noexcept { return dists[0][v]; }
    /**
     * @brief Returns the bounds of the given variable.
     *
     * @param v the variable to get the bounds of.
     * @return std::pair<INT_TYPE, INT_TYPE> the bounds of the variable.
     */
    [[nodiscard]] inline std::pair<INT_TYPE, INT_TYPE> bounds(VARIABLE_TYPE v) const noexcept { return {-dists[v][0], dists[0][v]}; }
    /**
     * @brief Returns the distance between the given variables.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @return std::pair<INT_TYPE, INT_TYPE> the distance between the variables.
     */
    [[nodiscard]] inline std::pair<INT_TYPE, INT_TYPE> distance(VARIABLE_TYPE from, VARIABLE_TYPE to) const noexcept { return {-dists[to][from], dists[from][to]}; }

    /**
     * @brief Returns the bounds of the given linear expression.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<INT_TYPE, INT_TYPE> the bounds of the linear expression.
     */
    [[nodiscard]] std::pair<INT_TYPE, INT_TYPE> bounds(const utils::lin &l) const noexcept;
    /**
     * @brief Returns the distance between the given linear expressions.
     *
     * @param from the linear expression to get the distance from.
     * @param to the linear expression to get the distance to.
     * @return std::pair<INT_TYPE, INT_TYPE> the distance between the linear expressions.
     */
    [[nodiscard]] std::pair<INT_TYPE, INT_TYPE> distance(const utils::lin &from, const utils::lin &to) const noexcept { return bounds(from - to); }

    /**
     * @brief Checks if the given linear expressions match.
     *
     * Two linear expressions match if they can be equal.
     *
     * @param l0 the first linear expression.
     * @param l1 the second linear expression.
     * @return true if the linear expressions match.
     * @return false otherwise.
     */
    bool matches(const utils::lin &l0, const utils::lin &l1) const;

#ifdef BUILD_LISTENERS
    void add_listener(idl_value_listener &l) noexcept;
    void remove_listener(idl_value_listener &l) noexcept;
#endif

  private:
    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    void propagate(VARIABLE_TYPE from, VARIABLE_TYPE to, INT_TYPE dist) noexcept;
    [[nodiscard]] bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

    void set_dist(VARIABLE_TYPE from, VARIABLE_TYPE to, INT_TYPE dist) noexcept;
    void set_pred(VARIABLE_TYPE from, VARIABLE_TYPE to, VARIABLE_TYPE pred) noexcept;

  private:
    /**
     * @brief Resize the distance and predecessor matrices.
     *
     * @param size the new size of the matrices.
     */
    void resize(const size_t &size) noexcept;

    struct layer
    {
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, INT_TYPE> old_dists;                                                               // the updated distances..
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, VARIABLE_TYPE> old_preds;                                                              // the updated predecessors..
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::optional<std::reference_wrapper<distance_constraint<INT_TYPE>>>> old_constrs; // the updated constraints..
    };

  private:
    size_t n_vars = 1;                                                                                                                      // the number of variables..
    std::vector<std::vector<INT_TYPE>> dists;                                                                                           // the distance matrix..
    std::vector<std::vector<VARIABLE_TYPE>> preds;                                                                                          // the predecessor matrix..
    std::unordered_map<VARIABLE_TYPE, std::unique_ptr<distance_constraint<INT_TYPE>>> var_dists;                                        // the constraints controlled by a propositional variable (for propagation purposes)..
    std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::vector<std::reference_wrapper<distance_constraint<INT_TYPE>>>> dist_constrs; // the constraints between two temporal points (for propagation purposes)..
    std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::reference_wrapper<distance_constraint<INT_TYPE>>> dist_constr;               // the currently enforced constraints..
    std::vector<layer> layers;                                                                                                              // we store the updates..
#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<VARIABLE_TYPE, std::set<idl_value_listener *>> listening; // for each variable, the listeners listening to it..
    std::set<idl_value_listener *> listeners;                                    // the collection of listeners..
#endif
  };
} // namespace semitone
