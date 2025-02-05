#pragma once

#include "term.hpp"
#include "lit.hpp"
#include "lin.hpp"
#include <map>

namespace semitone
{
  class network;

  class context
  {
    friend class network;

  public:
    context() = default;
    context(const context &) = delete;
    context(context &&) = default;

    [[nodiscard]] bool_expr mk_bool_var(std::string_view name);
    [[nodiscard]] bool_expr mk_bool_const(const utils::lbool value);

    [[nodiscard]] bool_expr mk_and(std::vector<bool_expr> &&args);
    [[nodiscard]] bool_expr mk_or(std::vector<bool_expr> &&args);
    [[nodiscard]] bool_expr mk_not(bool_expr arg);

    [[nodiscard]] int_expr mk_int_var(std::string_view name);
    [[nodiscard]] int_expr mk_int_var(std::string_view name, const utils::integer &lb, const utils::integer &ub);
    [[nodiscard]] int_expr mk_int_const(const utils::integer &value);

    [[nodiscard]] int_expr mk_sum(std::vector<int_expr> &&args);
    [[nodiscard]] int_expr mk_sub(std::vector<int_expr> &&args);
    [[nodiscard]] int_expr mk_mul(std::vector<int_expr> &&args);
    [[nodiscard]] int_expr mk_div(std::vector<int_expr> &&args);

    [[nodiscard]] bool_expr mk_lt(int_expr lhs, int_expr rhs);
    [[nodiscard]] bool_expr mk_le(int_expr lhs, int_expr rhs);
    [[nodiscard]] bool_expr mk_eq(int_expr lhs, int_expr rhs);
    [[nodiscard]] bool_expr mk_ge(int_expr lhs, int_expr rhs);
    [[nodiscard]] bool_expr mk_gt(int_expr lhs, int_expr rhs);

    [[nodiscard]] real_expr mk_real_var(std::string_view name);
    [[nodiscard]] real_expr mk_real_var(std::string_view name, const utils::rational &lb, const utils::rational &ub);
    [[nodiscard]] real_expr mk_real_const(const utils::rational &value);

    [[nodiscard]] real_expr mk_sum(std::vector<real_expr> &&args);
    [[nodiscard]] real_expr mk_sub(std::vector<real_expr> &&args);
    [[nodiscard]] real_expr mk_mul(std::vector<real_expr> &&args);
    [[nodiscard]] real_expr mk_div(std::vector<real_expr> &&args);

    [[nodiscard]] bool_expr mk_lt(real_expr lhs, real_expr rhs);
    [[nodiscard]] bool_expr mk_le(real_expr lhs, real_expr rhs);
    [[nodiscard]] bool_expr mk_eq(real_expr lhs, real_expr rhs);
    [[nodiscard]] bool_expr mk_ge(real_expr lhs, real_expr rhs);
    [[nodiscard]] bool_expr mk_gt(real_expr lhs, real_expr rhs);

  private:
    [[nodiscard]] bool_expr to_cnf(bool_expr expr);
    [[nodiscard]] bool_expr push_negations(bool_expr expr);
    [[nodiscard]] bool_expr distribute(bool_expr expr);

    [[nodiscard]] bool_expr simplify(bool_expr expr);
    [[nodiscard]] std::map<std::string, utils::integer> linearize(int_expr expr);
    [[nodiscard]] std::map<std::string, utils::rational> linearize(real_expr expr);

    [[nodiscard]] std::vector<utils::lit> to_lits(bool_expr expr);
    [[nodiscard]] utils::lin to_lin(int_expr expr);
    [[nodiscard]] utils::lin to_lin(real_expr expr);

  private:
    std::map<std::string, utils::var> var_map;
    std::map<std::string, utils::var> int_var_map;
    std::map<std::string, utils::var> real_var_map;
  };

  [[nodiscard]] bool_expr operator&&(bool_expr lhs, bool_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator&&(bool_expr lhs, bool rhs) noexcept;
  [[nodiscard]] bool_expr operator&&(bool lhs, bool_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator||(bool_expr lhs, bool_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator||(bool_expr lhs, bool rhs) noexcept;
  [[nodiscard]] bool_expr operator||(bool lhs, bool_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator!(bool_expr arg) noexcept;

  [[nodiscard]] int_expr operator+(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator+(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] int_expr operator+(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator+(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] int_expr operator+(int lhs, int_expr rhs) noexcept;

  [[nodiscard]] int_expr operator-(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator-(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] int_expr operator-(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator-(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] int_expr operator-(int lhs, int_expr rhs) noexcept;

  [[nodiscard]] int_expr operator*(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator*(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] int_expr operator*(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator*(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] int_expr operator*(int lhs, int_expr rhs) noexcept;

  [[nodiscard]] int_expr operator/(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator/(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] int_expr operator/(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] int_expr operator/(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] int_expr operator/(int lhs, int_expr rhs) noexcept;

  [[nodiscard]] bool_expr operator<(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] bool_expr operator<(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] bool_expr operator<(int lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(int lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] bool_expr operator==(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] bool_expr operator==(int lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(int lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(int_expr lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(int_expr lhs, utils::integer rhs) noexcept;
  [[nodiscard]] bool_expr operator>(utils::integer lhs, int_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(int_expr lhs, int rhs) noexcept;
  [[nodiscard]] bool_expr operator>(int lhs, int_expr rhs) noexcept;

  [[nodiscard]] real_expr operator+(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator+(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] real_expr operator+(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator+(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] real_expr operator+(double lhs, real_expr rhs) noexcept;

  [[nodiscard]] real_expr operator-(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator-(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] real_expr operator-(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator-(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] real_expr operator-(double lhs, real_expr rhs) noexcept;

  [[nodiscard]] real_expr operator*(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator*(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] real_expr operator*(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator*(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] real_expr operator*(double lhs, real_expr rhs) noexcept;

  [[nodiscard]] real_expr operator/(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator/(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] real_expr operator/(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] real_expr operator/(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] real_expr operator/(double lhs, real_expr rhs) noexcept;

  [[nodiscard]] bool_expr operator<(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] bool_expr operator<(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] bool_expr operator<(double lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] bool_expr operator<=(double lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] bool_expr operator==(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator==(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] bool_expr operator==(double lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] bool_expr operator>=(double lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(real_expr lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(real_expr lhs, utils::rational rhs) noexcept;
  [[nodiscard]] bool_expr operator>(utils::rational lhs, real_expr rhs) noexcept;
  [[nodiscard]] bool_expr operator>(real_expr lhs, double rhs) noexcept;
  [[nodiscard]] bool_expr operator>(double lhs, real_expr rhs) noexcept;
} // namespace semitone
