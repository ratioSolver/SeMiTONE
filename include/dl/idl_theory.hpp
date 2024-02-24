#pragma once

#include <vector>
#include "theory.hpp"

namespace semitone
{
  class idl_theory final : public theory
  {
  public:
    idl_theory(std::shared_ptr<sat_core> sat, const size_t &size = 16);

    /**
     * @brief Create a new difference logic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    VARIABLE_TYPE new_var() noexcept;

  private:
    /**
     * @brief Resize the distance and predecessor matrices.
     *
     * @param size the new size of the matrices.
     */
    void resize(const size_t &size) noexcept;

  private:
    size_t n_vars = 1;
    std::vector<std::vector<INTEGER_TYPE>> dists;  // the distance matrix..
    std::vector<std::vector<VARIABLE_TYPE>> preds; // the predecessor matrix..
  };
} // namespace semitone
