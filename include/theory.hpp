#pragma once

#include "lit.hpp"
#include <vector>

namespace semitone
{
  class network;

  class theory
  {
  public:
    theory(network &net) noexcept;
    virtual ~theory() noexcept = default;

  protected:
    void bind(const utils::var &v) noexcept;

  private:
    /**
     * @brief Propagate a literal.
     *
     * @param p The literal to propagate.
     */
    virtual bool propagate(const utils::lit &p) noexcept = 0;

  protected:
    network &net;
  };
} // namespace semitone
