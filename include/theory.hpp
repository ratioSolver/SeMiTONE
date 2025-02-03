#pragma once

#include "context.hpp"

namespace semitone
{
  class network;

  class theory
  {
  public:
    theory(network &slv) noexcept : slv(slv) {}

    virtual ~theory() noexcept = default;

  protected:
    network &slv;
  };
} // namespace semitone
