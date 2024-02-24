#pragma once

#include "theory.hpp"

namespace semitone
{
  class dl_theory final : public theory
  {
  public:
    dl_theory(std::shared_ptr<sat_core> sat);

    /**
     * @brief Create a new difference logic variable.
     */
    VARIABLE_TYPE new_var() noexcept;
  };
} // namespace semitone
