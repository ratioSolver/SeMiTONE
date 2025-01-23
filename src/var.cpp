#include "var.hpp"
#include <cassert>

namespace semitone
{
    var::var(const context &ctx) : ctx(ctx) {}

    const context &var::get_ctx() const noexcept { return ctx; }

    bool_var::bool_var(const context &ctx) : var(ctx), value(utils::Undefined) {}
    bool_var::bool_var(const context &ctx, utils::lbool val) : var(ctx), value(val) {}

    utils::lbool bool_var::val() const noexcept { return value; }

    and_expr::and_expr(const context &ctx, std::vector<bool_expr> &&args) : bool_var(ctx), arguments(std::move(args)) {}

    const std::vector<bool_expr> &and_expr::args() const noexcept { return arguments; }

    utils::lbool and_expr::val() const noexcept
    {
        for (const auto &arg : arguments)
        {
            if (arg->val() == utils::False)
                return utils::False;
            if (arg->val() == utils::Undefined)
                return utils::Undefined;
        }
        return utils::True;
    }

    or_expr::or_expr(const context &ctx, std::vector<bool_expr> &&args) : bool_var(ctx), arguments(std::move(args)) {}

    const std::vector<bool_expr> &or_expr::args() const noexcept { return arguments; }

    utils::lbool or_expr::val() const noexcept
    {
        for (const auto &arg : arguments)
        {
            if (arg->val() == utils::True)
                return utils::True;
            if (arg->val() == utils::Undefined)
                return utils::Undefined;
        }
        return utils::False;
    }

    not_expr::not_expr(const context &ctx, bool_expr arg) : bool_var(ctx), argument(std::move(arg)) {}

    bool_expr not_expr::arg() const noexcept { return argument; }

    utils::lbool not_expr::val() const noexcept
    {
        switch (argument->val())
        {
        case utils::True:
            return utils::False;
        case utils::False:
            return utils::True;
        default:
            return utils::Undefined;
        }
    }

    int_var::int_var(const context &ctx) : var(ctx), value(utils::integer::zero), lower_bound(utils::integer::negative_infinite), upper_bound(utils::integer::positive_infinite) {}
    int_var::int_var(const context &ctx, utils::integer val) : var(ctx), value(val), lower_bound(val), upper_bound(val) {}
    int_var::int_var(const context &ctx, utils::integer val, utils::integer lb, utils::integer ub) : var(ctx), value(val), lower_bound(lb), upper_bound(ub) { assert(lb <= val && val <= ub); }

    utils::integer int_var::lb() const noexcept { return lower_bound; }
    utils::integer int_var::ub() const noexcept { return upper_bound; }

    utils::integer int_var::val() const noexcept { return value; }

    real_var::real_var(const context &ctx) : var(ctx), value(utils::rational::zero), lower_bound(utils::rational::negative_infinite), upper_bound(utils::rational::positive_infinite) {}
    real_var::real_var(const context &ctx, utils::rational val) : var(ctx), value(val), lower_bound(val), upper_bound(val) {}
    real_var::real_var(const context &ctx, utils::rational val, utils::rational lb, utils::rational ub) : var(ctx), value(val), lower_bound(lb), upper_bound(ub) { assert(lb <= val && val <= ub); }

    utils::rational real_var::lb() const noexcept { return lower_bound; }
    utils::rational real_var::ub() const noexcept { return upper_bound; }

    utils::rational real_var::val() const noexcept { return value; }

    string_var::string_var(const context &ctx) : var(ctx), value("") {}
    string_var::string_var(const context &ctx, std::string val) : var(ctx), value(std::move(val)) {}

    std::string string_var::val() const noexcept { return value; }
} // namespace semitone
