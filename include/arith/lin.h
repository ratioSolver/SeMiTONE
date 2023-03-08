#pragma once

#include "semitone_export.h"
#include "defs.h"
#include "rational.h"
#include "json.h"
#include <map>

namespace semitone
{
  class lin final
  {
  public:
    /**
     * @brief Construct a new linear expression object.
     *
     */
    SEMITONE_EXPORT explicit lin();
    /**
     * @brief Construct a new linear expression object given the `known_term`.
     *
     * @param known_term the known term of the linear expression.
     */
    SEMITONE_EXPORT explicit lin(const utils::rational &known_term);
    /**
     * @brief Construct a new linear expression object given the `v` variable and the `c` coefficient.
     *
     * @param v the variable of the linear expression.
     * @param c the coefficient of the `v` variable.
     */
    SEMITONE_EXPORT explicit lin(const var v, const utils::rational &c);

  public:
    SEMITONE_EXPORT lin operator+(const lin &rhs) const noexcept;
    SEMITONE_EXPORT lin operator+(const utils::rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator+(const utils::rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator-(const lin &rhs) const noexcept;
    SEMITONE_EXPORT lin operator-(const utils::rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator-(const utils::rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator*(const utils::rational &rhs) const noexcept;
    SEMITONE_EXPORT friend lin operator*(const utils::rational &lhs, const lin &rhs) noexcept;

    SEMITONE_EXPORT lin operator/(const utils::rational &rhs) const noexcept;

    SEMITONE_EXPORT lin operator+=(const lin &rhs) noexcept;
    SEMITONE_EXPORT lin operator+=(const utils::rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator-=(const lin &rhs) noexcept;
    SEMITONE_EXPORT lin operator-=(const utils::rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator*=(const utils::rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator/=(const utils::rational &rhs) noexcept;

    SEMITONE_EXPORT lin operator-() const noexcept;

    SEMITONE_EXPORT friend std::string to_string(const lin &rhs) noexcept;

  public:
    std::map<const var, utils::rational> vars;
    utils::rational known_term;
  };
} // namespace semitone