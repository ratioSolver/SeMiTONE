#pragma once

#include "sat_core.h"
#include <memory>

namespace semitone
{
  class sat_stack final
  {
  public:
    SEMITONE_EXPORT sat_stack();
    sat_stack(const sat_stack &orig) = delete;

    SEMITONE_EXPORT void push() noexcept;
    SEMITONE_EXPORT void pop() noexcept;

    SEMITONE_EXPORT std::shared_ptr<sat_core> &top() noexcept { return stack.back(); }
    SEMITONE_EXPORT bool empty() const noexcept { return stack.empty(); }
    SEMITONE_EXPORT size_t size() const noexcept { return stack.size(); }

    SEMITONE_EXPORT sat_core &operator[](const size_t &idx) { return *stack[idx]; }

    inline lbool value(const var &x) const noexcept { return stack.back()->value(x); }
    inline lbool value(const lit &p) const noexcept { return stack.back()->value(p); }

  private:
    std::vector<std::shared_ptr<sat_core>> stack;
  };
} // namespace semitone