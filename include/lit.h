#pragma once

#include "semitone_export.h"
#include "defs.h"
#include <limits>
#include <string>

namespace semitone
{
  /**
   * This class is used for representing propositional literals.
   */
  class lit
  {
  public:
    explicit constexpr lit(var v = std::numeric_limits<var>::max(), bool sign = true) : x((v << 1) + sign) {}

    /**
     * @brief Get the variable of the literal.
     * 
     * @param p The literal.
     * @return var The variable of the literal.
     */
    inline friend var variable(const lit &p) noexcept { return p.x >> 1; }
    /**
     * @brief Get the sign of the literal.
     * 
     * @param p The literal.
     * @return bool The sign of the literal.
     */
    inline friend bool sign(const lit &p) noexcept { return p.x & 1; }
    
    inline friend size_t index(const lit &p) noexcept { return p.x; }
    inline friend bool is_undefined(const lit &p) noexcept { return p.x == std::numeric_limits<var>::max(); }

    inline constexpr lit operator!() const
    {
      lit p;
      p.x = x ^ 1;
      return p;
    }
    inline bool operator<(const lit &rhs) const noexcept { return x < rhs.x; }
    inline bool operator==(const lit &rhs) const noexcept { return x == rhs.x; }
    inline bool operator!=(const lit &rhs) const noexcept { return x != rhs.x; }

    friend std::string to_string(const lit &p) noexcept { return sign(p) ? ("b" + std::to_string(variable(p))) : ("¬b" + std::to_string(variable(p))); }

  private:
    size_t x;
  };

  constexpr lit FALSE_lit(FALSE_var);
  constexpr lit TRUE_lit = !FALSE_lit;
} // namespace semitone