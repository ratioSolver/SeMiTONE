#pragma once

#include "theory.hpp"

namespace semitone
{
  class lra_theory final : public theory
  {
  public:
    lra_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new linear real arithmetic variable.
     *
     * @return VARIABLE_TYPE the new variable.
     */
    VARIABLE_TYPE new_var() noexcept;
  };
} // namespace semitone
