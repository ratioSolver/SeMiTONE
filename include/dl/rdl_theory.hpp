#pragma once

#include <vector>
#include "theory.hpp"
#include "inf_rational.hpp"

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
