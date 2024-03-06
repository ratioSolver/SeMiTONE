#pragma once

#include <vector>
#include "theory.hpp"
#include "inf_rational.hpp"
#include "lin.hpp"

namespace semitone
{
  class rdl_theory final : public theory
  {
  public:
    rdl_theory(std::shared_ptr<sat_core> sat, const size_t &size = 16);

    /**
     * @brief Create a new difference logic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    [[nodiscard]] VARIABLE_TYPE new_var() noexcept;

    /**
     * @brief Returns the lower bound of the given variable.
     *
     * @param v the variable to get the lower bound of.
     * @return utils::inf_rational the lower bound of the variable.
     */
    inline utils::inf_rational lb(VARIABLE_TYPE v) const noexcept { return -dists[v][0]; }
    /**
     * @brief Returns the upper bound of the given variable.
     *
     * @param v the variable to get the upper bound of.
     * @return utils::inf_rational the upper bound of the variable.
     */
    inline utils::inf_rational ub(VARIABLE_TYPE v) const noexcept { return dists[0][v]; }
    /**
     * @brief Returns the bounds of the given variable.
     *
     * @param v the variable to get the bounds of.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the bounds of the variable.
     */
    inline std::pair<utils::inf_rational, utils::inf_rational> bounds(VARIABLE_TYPE v) const noexcept { return std::make_pair(-dists[v][0], dists[0][v]); }
    /**
     * @brief Returns the distance between the given variables.
     *
     * @param from the variable to start from.
     * @param to the variable to end at.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the distance between the variables.
     */
    inline std::pair<utils::inf_rational, utils::inf_rational> distance(VARIABLE_TYPE from, VARIABLE_TYPE to) const noexcept { return std::make_pair(-dists[to][from], dists[from][to]); }

    /**
     * @brief Returns the bounds of the given linear expression.
     *
     * @param l the linear expression to get the bounds of.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the bounds of the linear expression.
     */
    std::pair<utils::inf_rational, utils::inf_rational> bounds(const utils::lin &l) const;
    /**
     * @brief Returns the distance between the given linear expressions.
     *
     * @param from the linear expression to start from.
     * @param to the linear expression to end at.
     * @return std::pair<utils::inf_rational, utils::inf_rational> the distance between the linear expressions.
     */
    std::pair<utils::inf_rational, utils::inf_rational> distance(const utils::lin &from, const utils::lin &to) const { return bounds(from - to); }

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
    std::vector<std::vector<utils::inf_rational>> dists; // the distance matrix..
    std::vector<std::vector<VARIABLE_TYPE>> preds;       // the predecessor matrix..
  };
} // namespace semitone
