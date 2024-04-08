#pragma once

#include <vector>
#include "theory.hpp"
#include "dl_distance.hpp"
#include "lin.hpp"

namespace semitone
{
  class idl_theory final : public theory
  {
  public:
    idl_theory(std::shared_ptr<sat_core> sat, const size_t &size = 16) noexcept;

    /**
     * @brief Create a new difference logic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Creates a new distance between the given variables and returns the corresponding literal.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @param dist the distance between the variables.
     * @return utils::lit the literal corresponding to the distance.
     */
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INTEGER_TYPE dist) noexcept;
    /**
     * @brief Creates a new distance between the given variables and returns the corresponding literal.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @param min the minimum distance between the variables.
     * @param max the maximum distance between the variables.
     * @return utils::lit the literal corresponding to the distance.
     */
    [[nodiscard]] utils::lit new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INTEGER_TYPE min, INTEGER_TYPE max) noexcept;

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
     * @return std::pair<VARIABLE_TYPE, VARIABLE_TYPE> the bounds of the variable.
     */
    [[nodiscard]] inline std::pair<VARIABLE_TYPE, VARIABLE_TYPE> bounds(VARIABLE_TYPE v) const noexcept { return std::make_pair(-dists[v][0], dists[0][v]); }
    /**
     * @brief Returns the distance between the given variables.
     *
     * @param from the variable to get the distance from.
     * @param to the variable to get the distance to.
     * @return std::pair<VARIABLE_TYPE, VARIABLE_TYPE> the distance between the variables.
     */
    [[nodiscard]] inline std::pair<VARIABLE_TYPE, VARIABLE_TYPE> distance(VARIABLE_TYPE from, VARIABLE_TYPE to) const noexcept { return std::make_pair(-dists[to][from], dists[from][to]); }

    /**
     * @brief Returns the bounds of the given linear expression.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<VARIABLE_TYPE, VARIABLE_TYPE> the bounds of the linear expression.
     */
    [[nodiscard]] std::pair<VARIABLE_TYPE, VARIABLE_TYPE> bounds(const utils::lin &l) const noexcept;
    /**
     * @brief Returns the distance between the given linear expressions.
     *
     * @param from the linear expression to get the distance from.
     * @param to the linear expression to get the distance to.
     * @return std::pair<VARIABLE_TYPE, VARIABLE_TYPE> the distance between the linear expressions.
     */
    [[nodiscard]] std::pair<VARIABLE_TYPE, VARIABLE_TYPE> distance(const utils::lin &from, const utils::lin &to) const noexcept { return bounds(from - to); }

  private:
    bool propagate(const utils::lit &) noexcept override { return true; }
    bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

  private:
    /**
     * @brief Resize the distance and predecessor matrices.
     *
     * @param size the new size of the matrices.
     */
    void resize(const size_t &size) noexcept;

  private:
    size_t n_vars = 1;
    std::vector<std::vector<INTEGER_TYPE>> dists;                                            // the distance matrix..
    std::vector<std::vector<VARIABLE_TYPE>> preds;                                           // the predecessor matrix..
    std::unordered_map<VARIABLE_TYPE, std::unique_ptr<dl_distance<INTEGER_TYPE>>> var_dists; // the constraints controlled by a propositional variable (for propagation purposes)..
  };
} // namespace semitone
