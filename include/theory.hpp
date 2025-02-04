#pragma once

#include "context.hpp"
#include "lit.hpp"

namespace semitone
{
  class network;

  class theory
  {
  public:
    theory(network &slv) noexcept : slv(slv) {}
    virtual ~theory() noexcept = default;

    /**
     * @brief Notifies the theory that some information for subsequent backtracking might need to be stored.
     */
    virtual void push() noexcept = 0;

    /**
     * @brief Notifies the theory that a backtracking step is required.
     */
    virtual void pop() noexcept = 0;

  protected:
    network &slv;
  };
} // namespace semitone
