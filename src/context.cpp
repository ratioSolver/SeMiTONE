#include "context.hpp"
#include <cassert>

namespace semitone
{
    bool_expr context::mk_bool_var(std::string_view name) { return utils::make_s_ptr<bool_var>(*this, name); }
    bool_expr context::mk_bool_const(const utils::lbool value)
    {
        assert(!is_undefined(value));
        return utils::make_s_ptr<bool_var>(*this, value);
    }

    bool_expr context::mk_and(std::vector<bool_expr> &&args) { return utils::make_s_ptr<and_expr>(*this, std::move(args)); }
    bool_expr context::mk_or(std::vector<bool_expr> &&args) { return utils::make_s_ptr<or_expr>(*this, std::move(args)); }
    bool_expr context::mk_not(bool_expr arg) { return utils::make_s_ptr<not_expr>(*this, std::move(arg)); }

    int_expr context::mk_int_var(std::string_view name) { return utils::make_s_ptr<int_var>(*this, name); }
    int_expr context::mk_int_var(std::string_view name, const utils::integer &lb, const utils::integer &ub)
    {
        assert(lb <= ub);
        return utils::make_s_ptr<int_var>(*this, name, lb, ub);
    }
    int_expr context::mk_int_const(const utils::integer &value)
    {
        assert(!is_infinite(value));
        return utils::make_s_ptr<int_var>(*this, value);
    }

    int_expr context::mk_sum(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_sum>(*this, std::move(args)); }
    int_expr context::mk_sub(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_sub>(*this, std::move(args)); }
    int_expr context::mk_mul(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_mul>(*this, std::move(args)); }
    int_expr context::mk_div(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_div>(*this, std::move(args)); }

    real_expr context::mk_real_var(std::string_view name) { return utils::make_s_ptr<real_var>(*this, name); }
    real_expr context::mk_real_var(std::string_view name, const utils::rational &lb, const utils::rational &ub)
    {
        assert(lb <= ub);
        return utils::make_s_ptr<real_var>(*this, name, lb, ub);
    }
    real_expr context::mk_real_const(const utils::rational &value)
    {
        assert(!is_infinite(value));
        return utils::make_s_ptr<real_var>(*this, value);
    }

    real_expr context::mk_sum(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_sum>(*this, std::move(args)); }
    real_expr context::mk_sub(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_sub>(*this, std::move(args)); }
    real_expr context::mk_mul(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_mul>(*this, std::move(args)); }
    real_expr context::mk_div(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_div>(*this, std::move(args)); }
} // namespace semitone
