#pragma once

#include "theory.hpp"

namespace semitone
{
  class dl_theory : public theory
  {
  public:
    dl_theory(network &net) noexcept;

  private:
    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;
  };
} // namespace semitone
