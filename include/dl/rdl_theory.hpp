#pragma once

#include <vector>
#include "theory.hpp"
#include "dl_distance_constraint.hpp"
#include "inf_rational.hpp"
#include "lin.hpp"
#include "inf_rational.hpp"
#ifdef BUILD_LISTENERS
#include <set>
#endif

namespace semitone
{
#ifdef BUILD_LISTENERS
  class rdl_value_listener;
#endif

  class rdl_theory final : public theory
  {
#ifdef BUILD_LISTENERS
    friend class rdl_value_listener;
#endif

  public:
    rdl_theory(const size_t &size = 16) noexcept;
    rdl_theory(const rdl_theory &orig) noexcept;
    ~rdl_theory();

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
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept;
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
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &min, const utils::inf_rational &max) noexcept;

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
     * @return utils::inf_rational the lower bound of the variable.
     */
    [[nodiscard]] inline utils::inf_rational lb(VARIABLE_TYPE v) const noexcept { return -dists[v][0]; }
    /**
     * @brief Returns the upper bound of the given variable.
     *
     * @param v the variable to get the upper bound of.
     * @return utils::inf_rational the upper bound of the variable.
     */
    [[nodiscard]] inline utils::inf_rational ub(VARIABLE_TYPE v) const noexcept { return dists[0][v]; }
    /**
     * @brief Returns the bounds of the given variable.
     *
     * @param v the variable to get the bounds of.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the bounds of the variable.
     */
    [[nodiscard]] inline std::pair<utils::inf_rational, utils::inf_rational> bounds(VARIABLE_TYPE v) const noexcept { return {-dists[v][0], dists[0][v]}; }
    /**
     * @brief Returns the distance between the given variables.
     *
     * @param from the variable to start from.
     * @param to the variable to end at.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the distance between the variables.
     */
    [[nodiscard]] inline std::pair<utils::inf_rational, utils::inf_rational> distance(VARIABLE_TYPE from, VARIABLE_TYPE to) const noexcept { return {-dists[to][from], dists[from][to]}; }

    /**
     * @brief Returns the bounds of the given linear expression.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the bounds of the linear expression.
     */
    [[nodiscard]] std::pair<utils::inf_rational, utils::inf_rational> bounds(const utils::lin &l) const noexcept;
    /**
     * @brief Returns the distance between the given linear expressions.
     *
     * @param from the linear expression to start from.
     * @param to the linear expression to end at.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the distance between the linear expressions.
     */
    [[nodiscard]] std::pair<utils::inf_rational, utils::inf_rational> distance(const utils::lin &from, const utils::lin &to) const noexcept { return bounds(from - to); }

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
    [[nodiscard]] bool matches(const utils::lin &l0, const utils::lin &l1) const;

#ifdef BUILD_LISTENERS
    void add_listener(rdl_value_listener &l) noexcept;
    void remove_listener(rdl_value_listener &l) noexcept;
#endif

  private:
    [[nodiscard]] bool propagate(const utils::lit &) noexcept override;
    void propagate(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept;
    [[nodiscard]] bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

    void set_dist(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept;
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
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, utils::inf_rational> old_dists;                                                               // the updated distances..
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, VARIABLE_TYPE> old_preds;                                                                     // the updated predecessors..
      std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::optional<std::reference_wrapper<distance_constraint<utils::inf_rational>>>> old_constrs; // the updated constraints..
    };

  private:
    size_t n_vars = 1;                                                                                                                             // the number of variables..
    std::vector<std::vector<utils::inf_rational>> dists;                                                                                           // the distance matrix..
    std::vector<std::vector<VARIABLE_TYPE>> preds;                                                                                                 // the predecessor matrix..
    std::unordered_map<VARIABLE_TYPE, std::unique_ptr<distance_constraint<utils::inf_rational>>> var_dists;                                        // the constraints controlled by a propositional variable (for propagation purposes)..
    std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::vector<std::reference_wrapper<distance_constraint<utils::inf_rational>>>> dist_constrs; // the constraints between two temporal points (for propagation purposes)..
    std::map<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>, std::reference_wrapper<distance_constraint<utils::inf_rational>>> dist_constr;               // the currently enforced constraints..
    std::vector<layer> layers;                                                                                                                     // we store the updates..
#ifdef BUILD_LISTENERS
  private:
    std::unordered_map<VARIABLE_TYPE, std::set<rdl_value_listener *>> listening; // for each variable, the listeners listening to it..
    std::set<rdl_value_listener *> listeners;                                    // the collection of listeners..
#endif
  };
} // namespace semitone
