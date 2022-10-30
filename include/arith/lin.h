#pragma once

#include "rational.h"
#include <map>

namespace semitone
{
  class lin final
  {
  public:
    SEMITONE_EXPORT explicit lin();
    SEMITONE_EXPORT explicit lin(const rational &known_term);
    SEMITONE_EXPORT explicit lin(const var v, const rational &c);

  public:
    SEMITONE_EXPORT lin operator+(const lin &rhs) const noexcept;
    SEMITONE_EXPORT lin operator+(const rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator+(const rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator-(const lin &rhs) const noexcept;
    SEMITONE_EXPORT lin operator-(const rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator-(const rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator*(const rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator*(const rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator/(const rational &rhs) const noexcept;

    SEMITONE_EXPORT lin operator+=(const lin &rhs) noexcept;
    SEMITONE_EXPORT lin operator+=(const rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator-=(const lin &rhs) noexcept;
    SEMITONE_EXPORT lin operator-=(const rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator*=(const rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator/=(const rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator-() const noexcept;

    SEMITONE_EXPORT friend std::string to_string(const lin &rhs) noexcept;

  public:
    std::map<const var, rational> vars;
    rational known_term;
  };
} // namespace semitone