#include "term.hpp"
#include <cassert>

namespace semitone
{
    term::term(context &ctx, std::string_view name) : ctx(ctx), name(name) {}
    context &term::get_ctx() noexcept { return ctx; }
    const std::string &term::get_name() const noexcept { return name; }

    bool_term::bool_term(context &ctx, std::string_view name) : term(ctx, name) {}

    bool_const::bool_const(context &ctx, utils::lbool val) : bool_term(ctx, val == utils::True ? "true" : "false"), value(val) { assert(!is_undefined(val)); }
    utils::lbool bool_const::val() const noexcept { return value; }

    bool_var::bool_var(context &ctx, std::string_view name) : bool_term(ctx, name), value(utils::Undefined) {}
    utils::lbool bool_var::val() const noexcept { return value; }

    and_expr::and_expr(context &ctx, std::vector<bool_expr> &&args) : bool_term(ctx, get_name(args)), arguments(std::move(args)) {}
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
    std::string and_expr::get_name(const std::vector<bool_expr> &args)
    {
        std::string name = "(and ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    or_expr::or_expr(context &ctx, std::vector<bool_expr> &&args) : bool_term(ctx, get_name(args)), arguments(std::move(args)) {}
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
    std::string or_expr::get_name(const std::vector<bool_expr> &args)
    {
        std::string name = "(or ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    not_expr::not_expr(context &ctx, bool_expr arg) : bool_term(ctx, "(not " + arg->get_name() + ")"), argument(std::move(arg)) {}
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

    int_term::int_term(context &ctx, std::string_view name) : term(ctx, name) {}

    int_const::int_const(context &ctx, const utils::integer &val) : int_term(ctx, to_string(val)), value(val) { assert(!is_infinite(val)); }
    utils::integer int_const::lb() const noexcept { return value; }
    utils::integer int_const::ub() const noexcept { return value; }
    utils::integer int_const::val() const noexcept { return value; }

    int_var::int_var(context &ctx, std::string_view name) : int_term(ctx, name), value(utils::integer::zero), lower_bound(utils::integer::negative_infinite), upper_bound(utils::integer::positive_infinite) {}
    int_var::int_var(context &ctx, std::string_view name, const utils::integer &lb, const utils::integer &ub) : int_term(ctx, name), value(get_val(lb, ub)), lower_bound(lb), upper_bound(ub) {}
    utils::integer int_var::lb() const noexcept { return lower_bound; }
    utils::integer int_var::ub() const noexcept { return upper_bound; }
    utils::integer int_var::val() const noexcept { return value; }
    utils::integer int_var::get_val(const utils::integer &lb, const utils::integer &ub)
    {
        assert(lb <= ub);
        if (ub < 0)
            return ub;
        if (lb > 0)
            return lb;
        return utils::integer::zero;
    }

    int_sum::int_sum(context &ctx, std::vector<int_expr> &&args) : int_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<int_expr> &int_sum::args() const noexcept { return arguments; }
    utils::integer int_sum::lb() const noexcept
    {
        utils::integer sum = utils::integer::zero;
        for (const auto &arg : arguments)
            sum += arg->lb();
        return sum;
    }
    utils::integer int_sum::ub() const noexcept
    {
        utils::integer sum = utils::integer::zero;
        for (const auto &arg : arguments)
            sum += arg->ub();
        return sum;
    }
    utils::integer int_sum::val() const noexcept
    {
        utils::integer sum = utils::integer::zero;
        for (const auto &arg : arguments)
            sum += arg->val();
        return sum;
    }
    std::string int_sum::get_name(const std::vector<int_expr> &args)
    {
        std::string name = "(+ ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    int_sub::int_sub(context &ctx, std::vector<int_expr> &&args) : int_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<int_expr> &int_sub::args() const noexcept { return arguments; }
    utils::integer int_sub::lb() const noexcept
    {
        utils::integer sub = arguments[0]->lb();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->ub();
        return sub;
    }
    utils::integer int_sub::ub() const noexcept
    {
        utils::integer sub = arguments[0]->ub();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->lb();
        return sub;
    }
    utils::integer int_sub::val() const noexcept
    {
        utils::integer sub = arguments[0]->val();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->val();
        return sub;
    }
    std::string int_sub::get_name(const std::vector<int_expr> &args)
    {
        std::string name = "(- ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    int_mul::int_mul(context &ctx, std::vector<int_expr> &&args) : int_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<int_expr> &int_mul::args() const noexcept { return arguments; }
    utils::integer int_mul::lb() const noexcept
    {
        utils::integer mul = utils::integer::one;
        for (const auto &arg : arguments)
            mul *= arg->lb();
        return mul;
    }
    utils::integer int_mul::ub() const noexcept
    {
        utils::integer mul = utils::integer::one;
        for (const auto &arg : arguments)
            mul *= arg->ub();
        return mul;
    }
    utils::integer int_mul::val() const noexcept
    {
        utils::integer mul = utils::integer::one;
        for (const auto &arg : arguments)
            mul *= arg->val();
        return mul;
    }
    std::string int_mul::get_name(const std::vector<int_expr> &args)
    {
        std::string name = "(* ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    int_div::int_div(context &ctx, std::vector<int_expr> &&args) : int_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<int_expr> &int_div::args() const noexcept { return arguments; }
    utils::integer int_div::lb() const noexcept
    {
        utils::integer div = arguments[0]->lb();
        for (size_t i = 1; i < arguments.size(); ++i)
            if (arguments[i]->lb() > 0)
                div /= arguments[i]->ub();
            else if (arguments[i]->ub() < 0)
                div /= arguments[i]->lb();
            else
                div /= utils::integer::zero;
        return div;
    }
    utils::integer int_div::ub() const noexcept
    {
        utils::integer div = arguments[0]->ub();
        for (size_t i = 1; i < arguments.size(); ++i)
            div /= arguments[i]->lb();
        return div;
    }
    utils::integer int_div::val() const noexcept
    {
        utils::integer div = arguments[0]->val();
        for (size_t i = 1; i < arguments.size(); ++i)
            div /= arguments[i]->val();
        return div;
    }
    std::string int_div::get_name(const std::vector<int_expr> &args)
    {
        std::string name = "(/ ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    int_lt::int_lt(context &ctx, int_expr lhs, int_expr rhs) : bool_term(ctx, "(< " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool int_lt::val() const noexcept
    {
        if (lhs->ub() < rhs->lb())
            return utils::True;
        if (lhs->lb() >= rhs->ub())
            return utils::False;
        return utils::Undefined;
    }
    int_expr int_lt::left() const noexcept { return lhs; }
    int_expr int_lt::right() const noexcept { return rhs; }

    int_le::int_le(context &ctx, int_expr lhs, int_expr rhs) : bool_term(ctx, "(<= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool int_le::val() const noexcept
    {
        if (lhs->ub() <= rhs->lb())
            return utils::True;
        if (lhs->lb() > rhs->ub())
            return utils::False;
        return utils::Undefined;
    }
    int_expr int_le::left() const noexcept { return lhs; }
    int_expr int_le::right() const noexcept { return rhs; }

    int_eq::int_eq(context &ctx, int_expr lhs, int_expr rhs) : bool_term(ctx, "(= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool int_eq::val() const noexcept
    {
        if (lhs->ub() < rhs->lb() || lhs->lb() > rhs->ub())
            return utils::False;
        if (lhs->lb() == rhs->lb() && lhs->ub() == rhs->ub() && lhs->lb() == rhs->ub())
            return utils::True;
        return utils::Undefined;
    }
    int_expr int_eq::left() const noexcept { return lhs; }
    int_expr int_eq::right() const noexcept { return rhs; }

    int_ge::int_ge(context &ctx, int_expr lhs, int_expr rhs) : bool_term(ctx, "(>= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool int_ge::val() const noexcept
    {
        if (lhs->lb() >= rhs->ub())
            return utils::True;
        if (lhs->ub() < rhs->lb())
            return utils::False;
        return utils::Undefined;
    }
    int_expr int_ge::left() const noexcept { return lhs; }
    int_expr int_ge::right() const noexcept { return rhs; }

    int_gt::int_gt(context &ctx, int_expr lhs, int_expr rhs) : bool_term(ctx, "(> " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool int_gt::val() const noexcept
    {
        if (lhs->lb() > rhs->ub())
            return utils::True;
        if (lhs->ub() <= rhs->lb())
            return utils::False;
        return utils::Undefined;
    }
    int_expr int_gt::left() const noexcept { return lhs; }
    int_expr int_gt::right() const noexcept { return rhs; }

    real_term::real_term(context &ctx, std::string_view name) : term(ctx, name) {}

    real_const::real_const(context &ctx, const utils::rational &val) : real_term(ctx, to_string(val)), value(val) { assert(!is_infinite(val)); }
    utils::rational real_const::lb() const noexcept { return value; }
    utils::rational real_const::ub() const noexcept { return value; }
    utils::rational real_const::val() const noexcept { return value; }

    real_var::real_var(context &ctx, std::string_view name) : real_term(ctx, name), value(utils::rational::zero), lower_bound(utils::rational::negative_infinite), upper_bound(utils::rational::positive_infinite) {}
    real_var::real_var(context &ctx, std::string_view name, const utils::rational &lb, const utils::rational &ub) : real_term(ctx, name), value(get_val(lb, ub)), lower_bound(lb), upper_bound(ub) {}
    utils::rational real_var::lb() const noexcept { return lower_bound; }
    utils::rational real_var::ub() const noexcept { return upper_bound; }
    utils::rational real_var::val() const noexcept { return value; }
    utils::rational real_var::get_val(const utils::rational &lb, const utils::rational &ub)
    {
        assert(lb <= ub);
        if (ub < 0)
            return ub;
        if (lb > 0)
            return lb;
        return utils::rational::zero;
    }

    real_sum::real_sum(context &ctx, std::vector<real_expr> &&args) : real_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<real_expr> &real_sum::args() const noexcept { return arguments; }
    utils::rational real_sum::lb() const noexcept
    {
        utils::rational sum = utils::rational::zero;
        for (const auto &arg : arguments)
            sum += arg->lb();
        return sum;
    }
    utils::rational real_sum::ub() const noexcept
    {
        utils::rational sum = utils::rational::zero;
        for (const auto &arg : arguments)
            sum += arg->ub();
        return sum;
    }
    utils::rational real_sum::val() const noexcept
    {
        utils::rational sum = utils::rational::zero;
        for (const auto &arg : arguments)
            sum += arg->val();
        return sum;
    }
    std::string real_sum::get_name(const std::vector<real_expr> &args)
    {
        std::string name = "(+ ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    real_sub::real_sub(context &ctx, std::vector<real_expr> &&args) : real_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<real_expr> &real_sub::args() const noexcept { return arguments; }
    utils::rational real_sub::lb() const noexcept
    {
        utils::rational sub = arguments[0]->lb();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->ub();
        return sub;
    }
    utils::rational real_sub::ub() const noexcept
    {
        utils::rational sub = arguments[0]->ub();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->lb();
        return sub;
    }
    utils::rational real_sub::val() const noexcept
    {
        utils::rational sub = arguments[0]->val();
        for (size_t i = 1; i < arguments.size(); ++i)
            sub -= arguments[i]->val();
        return sub;
    }
    std::string real_sub::get_name(const std::vector<real_expr> &args)
    {
        std::string name = "(- ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    real_mul::real_mul(context &ctx, std::vector<real_expr> &&args) : real_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<real_expr> &real_mul::args() const noexcept { return arguments; }
    utils::rational real_mul::lb() const noexcept
    {
        utils::rational mul = utils::rational::one;
        for (const auto &arg : arguments)
            mul *= arg->lb();
        return mul;
    }
    utils::rational real_mul::ub() const noexcept
    {
        utils::rational mul = utils::rational::one;
        for (const auto &arg : arguments)
            mul *= arg->ub();
        return mul;
    }
    utils::rational real_mul::val() const noexcept
    {
        utils::rational mul = utils::rational::one;
        for (const auto &arg : arguments)
            mul *= arg->val();
        return mul;
    }
    std::string real_mul::get_name(const std::vector<real_expr> &args)
    {
        std::string name = "(* ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    real_div::real_div(context &ctx, std::vector<real_expr> &&args) : real_term(ctx, get_name(args)), arguments(std::move(args)) {}
    const std::vector<real_expr> &real_div::args() const noexcept { return arguments; }
    utils::rational real_div::lb() const noexcept
    {
        utils::rational div = arguments[0]->lb();
        for (size_t i = 1; i < arguments.size(); ++i)
            div /= arguments[i]->ub();
        return div;
    }
    utils::rational real_div::ub() const noexcept
    {
        utils::rational div = arguments[0]->ub();
        for (size_t i = 1; i < arguments.size(); ++i)
            div /= arguments[i]->lb();
        return div;
    }
    utils::rational real_div::val() const noexcept
    {
        utils::rational div = arguments[0]->val();
        for (size_t i = 1; i < arguments.size(); ++i)
            div /= arguments[i]->val();
        return div;
    }
    std::string real_div::get_name(const std::vector<real_expr> &args)
    {
        std::string name = "(/ ";
        for (const auto &arg : args)
            name += arg->get_name() + " ";
        name.pop_back();
        name.push_back(')');
        return name;
    }

    real_lt::real_lt(context &ctx, real_expr lhs, real_expr rhs) : bool_term(ctx, "(< " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool real_lt::val() const noexcept
    {
        if (lhs->ub() < rhs->lb())
            return utils::True;
        if (lhs->lb() >= rhs->ub())
            return utils::False;
        return utils::Undefined;
    }
    real_expr real_lt::left() const noexcept { return lhs; }
    real_expr real_lt::right() const noexcept { return rhs; }

    real_le::real_le(context &ctx, real_expr lhs, real_expr rhs) : bool_term(ctx, "(<= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool real_le::val() const noexcept
    {
        if (lhs->ub() <= rhs->lb())
            return utils::True;
        if (lhs->lb() > rhs->ub())
            return utils::False;
        return utils::Undefined;
    }
    real_expr real_le::left() const noexcept { return lhs; }
    real_expr real_le::right() const noexcept { return rhs; }

    real_eq::real_eq(context &ctx, real_expr lhs, real_expr rhs) : bool_term(ctx, "(= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool real_eq::val() const noexcept
    {
        if (lhs->ub() < rhs->lb() || lhs->lb() > rhs->ub())
            return utils::False;
        if (lhs->lb() == rhs->lb() && lhs->ub() == rhs->ub() && lhs->lb() == rhs->ub())
            return utils::True;
        return utils::Undefined;
    }
    real_expr real_eq::left() const noexcept { return lhs; }
    real_expr real_eq::right() const noexcept { return rhs; }

    real_ge::real_ge(context &ctx, real_expr lhs, real_expr rhs) : bool_term(ctx, "(>= " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool real_ge::val() const noexcept
    {
        if (lhs->lb() >= rhs->ub())
            return utils::True;
        if (lhs->ub() < rhs->lb())
            return utils::False;
        return utils::Undefined;
    }
    real_expr real_ge::left() const noexcept { return lhs; }
    real_expr real_ge::right() const noexcept { return rhs; }

    real_gt::real_gt(context &ctx, real_expr lhs, real_expr rhs) : bool_term(ctx, "(> " + lhs->get_name() + " " + rhs->get_name() + ")"), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    utils::lbool real_gt::val() const noexcept
    {
        if (lhs->lb() > rhs->ub())
            return utils::True;
        if (lhs->ub() <= rhs->lb())
            return utils::False;
        return utils::Undefined;
    }
    real_expr real_gt::left() const noexcept { return lhs; }
    real_expr real_gt::right() const noexcept { return rhs; }

    string_var::string_var(context &ctx, std::string val) : term(ctx, val) {}
    std::string string_var::val() const noexcept { return "\"" + get_name() + "\""; }
} // namespace semitone
