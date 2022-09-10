#pragma once

#include "semitone_export.h"
#include "defs.h"
#include "json.h"
#include <string>

namespace semitone
{
  class rational
  {
  public:
    SEMITONE_EXPORT static const rational ZERO;
    SEMITONE_EXPORT static const rational ONE;
    SEMITONE_EXPORT static const rational POSITIVE_INFINITY;
    SEMITONE_EXPORT static const rational NEGATIVE_INFINITY;

    SEMITONE_EXPORT explicit rational();
    SEMITONE_EXPORT explicit rational(I n);
    SEMITONE_EXPORT explicit rational(I n, I d);

    inline I numerator() const noexcept { return num; }
    inline I denominator() const noexcept { return den; }

    inline friend bool is_integer(const rational &rhs) noexcept { return rhs.den == 1; }
    inline friend bool is_zero(const rational &rhs) noexcept { return rhs.num == 0; }
    inline friend bool is_positive(const rational &rhs) noexcept { return rhs.num > 0; }
    inline friend bool is_positive_or_zero(const rational &rhs) noexcept { return rhs.num >= 0; }
    inline friend bool is_negative(const rational &rhs) noexcept { return rhs.num < 0; }
    inline friend bool is_negative_or_zero(const rational &rhs) noexcept { return rhs.num <= 0; }
    inline friend bool is_infinite(const rational &rhs) noexcept { return rhs.den == 0; }
    inline friend bool is_positive_infinite(const rational &rhs) noexcept { return is_positive(rhs) && is_infinite(rhs); }
    inline friend bool is_negative_infinite(const rational &rhs) noexcept { return is_negative(rhs) && is_infinite(rhs); }
    inline friend double to_double(const rational &rhs) noexcept { return static_cast<double>(rhs.num) / static_cast<double>(rhs.den); }

    SEMITONE_EXPORT bool operator!=(const rational &rhs) const noexcept;
    SEMITONE_EXPORT bool operator<(const rational &rhs) const noexcept;
    SEMITONE_EXPORT bool operator<=(const rational &rhs) const noexcept;
    SEMITONE_EXPORT bool operator==(const rational &rhs) const noexcept;
    SEMITONE_EXPORT bool operator>=(const rational &rhs) const noexcept;
    SEMITONE_EXPORT bool operator>(const rational &rhs) const noexcept;

    SEMITONE_EXPORT bool operator!=(const I &rhs) const noexcept;
    SEMITONE_EXPORT bool operator<(const I &rhs) const noexcept;
    SEMITONE_EXPORT bool operator<=(const I &rhs) const noexcept;
    SEMITONE_EXPORT bool operator==(const I &rhs) const noexcept;
    SEMITONE_EXPORT bool operator>=(const I &rhs) const noexcept;
    SEMITONE_EXPORT bool operator>(const I &rhs) const noexcept;

    SEMITONE_EXPORT rational operator+(const rational &rhs) const noexcept;
    SEMITONE_EXPORT rational operator-(const rational &rhs) const noexcept;
    SEMITONE_EXPORT rational operator*(const rational &rhs) const noexcept;
    SEMITONE_EXPORT rational operator/(const rational &rhs) const noexcept;

    SEMITONE_EXPORT rational operator+(const I &rhs) const noexcept;
    SEMITONE_EXPORT rational operator-(const I &rhs) const noexcept;
    SEMITONE_EXPORT rational operator*(const I &rhs) const noexcept;
    SEMITONE_EXPORT rational operator/(const I &rhs) const noexcept;

    SEMITONE_EXPORT rational &operator+=(const rational &rhs) noexcept;
    SEMITONE_EXPORT rational &operator-=(const rational &rhs) noexcept;
    SEMITONE_EXPORT rational &operator*=(const rational &rhs) noexcept;
    SEMITONE_EXPORT rational &operator/=(const rational &rhs) noexcept;

    SEMITONE_EXPORT rational &operator+=(const I &rhs) noexcept;
    SEMITONE_EXPORT rational &operator-=(const I &rhs) noexcept;
    SEMITONE_EXPORT rational &operator*=(const I &rhs) noexcept;
    SEMITONE_EXPORT rational &operator/=(const I &rhs) noexcept;

    SEMITONE_EXPORT friend rational operator+(const I &lhs, const rational &rhs) noexcept;
    SEMITONE_EXPORT friend rational operator-(const I &lhs, const rational &rhs) noexcept;
    SEMITONE_EXPORT friend rational operator*(const I &lhs, const rational &rhs) noexcept;
    SEMITONE_EXPORT friend rational operator/(const I &lhs, const rational &rhs) noexcept;

    SEMITONE_EXPORT rational operator-() const noexcept;

  private:
    void normalize() noexcept;

    friend SEMITONE_EXPORT std::string to_string(const rational &rhs) noexcept;
    friend json::json to_json(const rational &rhs) noexcept
    {
      json::json j_rat;
      j_rat["num"] = rhs.num;
      j_rat["den"] = rhs.den;
      return j_rat;
    }

  private:
    I num; // the numerator..
    I den; // the denominator..
  };
} // namespace semitone