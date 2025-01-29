#include "var.hpp"

namespace semitone
{
  class context
  {
  public:
    context() = default;
    context(const context &) = delete;
    context(context &&) = default;

    bool_expr mk_bool_var(std::string_view name);
    bool_expr mk_bool_const(const utils::lbool value);

    bool_expr mk_and(std::vector<bool_expr> &&args);
    bool_expr mk_or(std::vector<bool_expr> &&args);
    bool_expr mk_not(bool_expr arg);

    int_expr mk_int_var(std::string_view name);
    int_expr mk_int_var(std::string_view name, const utils::integer &lb, const utils::integer &ub);
    int_expr mk_int_const(const utils::integer &value);

    int_expr mk_sum(std::vector<int_expr> &&args);
    int_expr mk_sub(std::vector<int_expr> &&args);
    int_expr mk_mul(std::vector<int_expr> &&args);
    int_expr mk_div(std::vector<int_expr> &&args);

    bool_expr mk_lt(int_expr lhs, int_expr rhs);
    bool_expr mk_le(int_expr lhs, int_expr rhs);
    bool_expr mk_eq(int_expr lhs, int_expr rhs);
    bool_expr mk_ge(int_expr lhs, int_expr rhs);
    bool_expr mk_gt(int_expr lhs, int_expr rhs);

    real_expr mk_real_var(std::string_view name);
    real_expr mk_real_var(std::string_view name, const utils::rational &lb, const utils::rational &ub);
    real_expr mk_real_const(const utils::rational &value);

    real_expr mk_sum(std::vector<real_expr> &&args);
    real_expr mk_sub(std::vector<real_expr> &&args);
    real_expr mk_mul(std::vector<real_expr> &&args);
    real_expr mk_div(std::vector<real_expr> &&args);

    bool_expr mk_lt(real_expr lhs, real_expr rhs);
    bool_expr mk_le(real_expr lhs, real_expr rhs);
    bool_expr mk_eq(real_expr lhs, real_expr rhs);
    bool_expr mk_ge(real_expr lhs, real_expr rhs);
    bool_expr mk_gt(real_expr lhs, real_expr rhs);
  };
} // namespace semitone
