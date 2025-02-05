#pragma once

#include "lit.hpp"
#include <vector>

namespace semitone
{
  class network;

  class theory
  {
  public:
    theory(network &net) noexcept : net(net) {}
    virtual ~theory() noexcept = default;

  private:
    /**
     * @brief Propagate a literal.
     *
     * @param p The literal to propagate.
     */
    virtual void propagate(const utils::lit &p) noexcept = 0;

  protected:
    network &net;
  };
} // namespace semitone
