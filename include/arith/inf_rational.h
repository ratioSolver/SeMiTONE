#pragma once

#include "rational.h"

namespace semitone
{
  class inf_rational
  {
  public:
    explicit inf_rational() = default;
    explicit inf_rational(I nun) : rat(nun) {}
    explicit inf_rational(const rational &rat) : rat(rat) {}
    explicit inf_rational(I nun, I den) : rat(nun, den) {}
    explicit inf_rational(const rational &rat, I inf) : rat(rat), inf(inf) {}
    explicit inf_rational(const rational &rat, const rational &inf) : rat(rat), inf(inf) {}

    inline rational get_rational() const noexcept { return rat; }
    inline rational get_infinitesimal() const noexcept { return inf; }

    inline friend bool is_zero(const inf_rational &rhs) noexcept { return is_zero(rhs.rat) && is_zero(rhs.inf); }
    inline friend bool is_positive(const inf_rational &rhs) noexcept { return is_positive(rhs.rat) || (is_zero(rhs.rat) && is_positive(rhs.inf)); }
    inline friend bool is_positive_or_zero(const inf_rational &rhs) noexcept { return is_positive(rhs.rat) || (is_zero(rhs.rat) && is_positive_or_zero(rhs.inf)); }
    inline friend bool is_negative(const inf_rational &rhs) noexcept { return is_negative(rhs.rat) || (is_zero(rhs.rat) && is_negative(rhs.inf)); }
    inline friend bool is_negative_or_zero(const inf_rational &rhs) noexcept { return is_negative(rhs.rat) || (is_zero(rhs.rat) && is_negative_or_zero(rhs.inf)); }
    inline friend bool is_infinite(const inf_rational &rhs) noexcept { return is_infinite(rhs.rat); }
    inline friend bool is_positive_infinite(const inf_rational &rhs) noexcept { return is_positive(rhs) && is_infinite(rhs); }
    inline friend bool is_negative_infinite(const inf_rational &rhs) noexcept { return is_negative(rhs) && is_infinite(rhs); }

    inline bool operator!=(const inf_rational &rhs) const noexcept { return rat != rhs.rat || inf != rhs.inf; };
    inline bool operator<(const inf_rational &rhs) const noexcept { return rat < rhs.rat || (rat == rhs.rat && inf < rhs.inf); };
    inline bool operator<=(const inf_rational &rhs) const noexcept { return rat < rhs.rat || (rat == rhs.rat && inf <= rhs.inf); };
    inline bool operator==(const inf_rational &rhs) const noexcept { return rat == rhs.rat && inf == rhs.inf; };
    inline bool operator>=(const inf_rational &rhs) const noexcept { return rat > rhs.rat || (rat == rhs.rat && inf >= rhs.inf); };
    inline bool operator>(const inf_rational &rhs) const noexcept { return rat > rhs.rat || (rat == rhs.rat && inf > rhs.inf); };

    inline bool operator!=(const rational &rhs) const noexcept { return rat != rhs || !is_zero(inf); };
    inline bool operator<(const rational &rhs) const noexcept { return rat < rhs || (rat == rhs && is_negative(inf)); };
    inline bool operator<=(const rational &rhs) const noexcept { return rat < rhs || (rat == rhs && is_negative_or_zero(inf)); };
    inline bool operator==(const rational &rhs) const noexcept { return rat == rhs && is_zero(inf); };
    inline bool operator>=(const rational &rhs) const noexcept { return rat > rhs || (rat == rhs && is_positive_or_zero(inf)); };
    inline bool operator>(const rational &rhs) const noexcept { return rat > rhs || (rat == rhs && is_positive(inf)); };

    inline bool operator!=(const I &rhs) const noexcept { return rat != rhs || !is_zero(inf); };
    inline bool operator<(const I &rhs) const noexcept { return rat < rhs || (rat == rhs && is_negative(inf)); };
    inline bool operator<=(const I &rhs) const noexcept { return rat < rhs || (rat == rhs && is_negative_or_zero(inf)); };
    inline bool operator==(const I &rhs) const noexcept { return rat == rhs && is_zero(inf); };
    inline bool operator>=(const I &rhs) const noexcept { return rat > rhs || (rat == rhs && is_positive_or_zero(inf)); };
    inline bool operator>(const I &rhs) const noexcept { return rat > rhs || (rat == rhs && is_positive(inf)); };

    inline inf_rational operator+(const inf_rational &rhs) const noexcept { return inf_rational(rat + rhs.rat, inf + rhs.inf); };
    inline inf_rational operator-(const inf_rational &rhs) const noexcept { return inf_rational(rat - rhs.rat, inf - rhs.inf); };

    inline inf_rational operator+(const rational &rhs) const noexcept { return inf_rational(rat + rhs, inf); };
    inline inf_rational operator-(const rational &rhs) const noexcept { return inf_rational(rat - rhs, inf); };
    inline inf_rational operator*(const rational &rhs) const noexcept { return inf_rational(rat * rhs, inf * rhs); };
    inline inf_rational operator/(const rational &rhs) const noexcept { return inf_rational(rat / rhs, inf / rhs); };

    inline inf_rational operator+(const I &rhs) const noexcept { return inf_rational(rat + rhs, inf); };
    inline inf_rational operator-(const I &rhs) const noexcept { return inf_rational(rat - rhs, inf); };
    inline inf_rational operator*(const I &rhs) const noexcept { return inf_rational(rat * rhs, inf * rhs); };
    inline inf_rational operator/(const I &rhs) const noexcept { return inf_rational(rat / rhs, inf / rhs); };

    inline inf_rational &operator+=(const inf_rational &rhs) noexcept
    {
      rat += rhs.rat;
      inf += rhs.inf;
      return *this;
    }
    inline inf_rational &operator-=(const inf_rational &rhs) noexcept
    {
      rat -= rhs.rat;
      inf -= rhs.inf;
      return *this;
    }

    inline inf_rational &operator+=(const rational &rhs) noexcept
    {
      rat += rhs;
      return *this;
    }
    inline inf_rational &operator-=(const rational &rhs) noexcept
    {
      rat -= rhs;
      return *this;
    }
    inline inf_rational &operator*=(const rational &rhs) noexcept
    {
      rat *= rhs;
      inf *= rhs;
      return *this;
    }
    inline inf_rational &operator/=(const rational &rhs) noexcept
    {
      rat /= rhs;
      inf /= rhs;
      return *this;
    }

    inline inf_rational &operator+=(const I &rhs) noexcept
    {
      rat += rhs;
      return *this;
    }
    inline inf_rational &operator-=(const I &rhs) noexcept
    {
      rat -= rhs;
      return *this;
    }
    inline inf_rational &operator*=(const I &rhs) noexcept
    {
      rat *= rhs;
      inf *= rhs;
      return *this;
    }
    inline inf_rational &operator/=(const I &rhs) noexcept
    {
      rat /= rhs;
      inf /= rhs;
      return *this;
    }

    inline inf_rational operator-() const noexcept { return inf_rational(-rat, -inf); }

    inline friend inf_rational operator+(const rational &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs + rhs.rat, rhs.inf); }
    inline friend inf_rational operator-(const rational &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs - rhs.rat, rhs.inf); }
    inline friend inf_rational operator*(const rational &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs * rhs.rat, lhs * rhs.inf); }
    inline friend inf_rational operator/(const rational &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs / rhs.rat, lhs / rhs.inf); }

    inline friend inf_rational operator+(const I &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs + rhs.rat, rhs.inf); }
    inline friend inf_rational operator-(const I &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs - rhs.rat, rhs.inf); }
    inline friend inf_rational operator*(const I &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs * rhs.rat, lhs * rhs.inf); }
    inline friend inf_rational operator/(const I &lhs, const inf_rational &rhs) noexcept { return inf_rational(lhs / rhs.rat, lhs / rhs.inf); }

    friend std::string to_string(const inf_rational &rhs) noexcept
    {
      if (is_infinite(rhs.rat) || rhs.inf == rational::ZERO)
        return to_string(rhs.rat);
      std::string c_str;
      if (rhs.rat != rational::ZERO)
        c_str += to_string(rhs.rat);
      if (rhs.inf == rational::ONE)
        if (c_str.empty())
          c_str += "ε";
        else
          c_str += " + ε";
      else if (rhs.inf == -rational::ONE)
        if (c_str.empty())
          c_str += "-ε";
        else
          c_str += " - ε";
      else if (is_negative(rhs.inf))
        if (c_str.empty())
          c_str += to_string(rhs.inf) + "ε";
        else
          c_str += " " + to_string(rhs.inf) + "ε";
      else if (c_str.empty())
        c_str += to_string(rhs.inf) + "ε";
      else
        c_str += " +" + to_string(rhs.inf) + "ε";
      return c_str;
    };

  private:
    rational rat; // the rational part..
    rational inf; // the infinitesimal part..
  };
} // namespace semitone