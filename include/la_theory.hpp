#pragma once

#include "theory.hpp"
#include "lin.hpp"

namespace semitone
{
  class la_theory : public theory
  {
  public:
    la_theory(network &net) noexcept;

    [[nodiscard]] utils::var new_int() noexcept;
    [[nodiscard]] utils::var new_real() noexcept;

    void add_lt(utils::lin &&lhs, utils::lin &&rhs) noexcept;
    [[nodiscard]] utils::lit new_lt(utils::lin &&lhs, utils::lin &&rhs) noexcept;
    void add_le(utils::lin &&lhs, utils::lin &&rhs) noexcept;
    [[nodiscard]] utils::lit new_le(utils::lin &&lhs, utils::lin &&rhs) noexcept;
    void add_eq(utils::lin &&lhs, utils::lin &&rhs) noexcept;
    [[nodiscard]] utils::lit new_eq(utils::lin &&lhs, utils::lin &&rhs) noexcept;

  private:
    void propagate(const utils::lit &p) noexcept override;
  };
} // namespace semitone
