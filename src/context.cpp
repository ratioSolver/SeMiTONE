#include "context.hpp"
#include <cassert>
#include <stdexcept>

namespace semitone
{
    bool_expr context::mk_bool_var(std::string_view name) { return utils::make_s_ptr<bool_var>(*this, name); }
    bool_expr context::mk_bool_const(const utils::lbool value)
    {
        assert(!is_undefined(value));
        return utils::make_s_ptr<bool_const>(*this, value);
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
        return utils::make_s_ptr<int_const>(*this, value);
    }

    int_expr context::mk_sum(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_sum>(*this, std::move(args)); }
    int_expr context::mk_sub(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_sub>(*this, std::move(args)); }
    int_expr context::mk_mul(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_mul>(*this, std::move(args)); }
    int_expr context::mk_div(std::vector<int_expr> &&args) { return utils::make_s_ptr<int_div>(*this, std::move(args)); }

    bool_expr context::mk_lt(int_expr lhs, int_expr rhs) { return utils::make_s_ptr<int_lt>(*this, lhs, rhs); }
    bool_expr context::mk_le(int_expr lhs, int_expr rhs) { return utils::make_s_ptr<int_le>(*this, lhs, rhs); }
    bool_expr context::mk_eq(int_expr lhs, int_expr rhs) { return utils::make_s_ptr<int_eq>(*this, lhs, rhs); }
    bool_expr context::mk_ge(int_expr lhs, int_expr rhs) { return utils::make_s_ptr<int_ge>(*this, lhs, rhs); }
    bool_expr context::mk_gt(int_expr lhs, int_expr rhs) { return utils::make_s_ptr<int_gt>(*this, lhs, rhs); }

    real_expr context::mk_real_var(std::string_view name) { return utils::make_s_ptr<real_var>(*this, name); }
    real_expr context::mk_real_var(std::string_view name, const utils::rational &lb, const utils::rational &ub)
    {
        assert(lb <= ub);
        return utils::make_s_ptr<real_var>(*this, name, lb, ub);
    }
    real_expr context::mk_real_const(const utils::rational &value)
    {
        assert(!is_infinite(value));
        return utils::make_s_ptr<real_const>(*this, value);
    }

    real_expr context::mk_sum(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_sum>(*this, std::move(args)); }
    real_expr context::mk_sub(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_sub>(*this, std::move(args)); }
    real_expr context::mk_mul(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_mul>(*this, std::move(args)); }
    real_expr context::mk_div(std::vector<real_expr> &&args) { return utils::make_s_ptr<real_div>(*this, std::move(args)); }

    bool_expr context::mk_lt(real_expr lhs, real_expr rhs) { return utils::make_s_ptr<real_lt>(*this, lhs, rhs); }
    bool_expr context::mk_le(real_expr lhs, real_expr rhs) { return utils::make_s_ptr<real_le>(*this, lhs, rhs); }
    bool_expr context::mk_eq(real_expr lhs, real_expr rhs) { return utils::make_s_ptr<real_eq>(*this, lhs, rhs); }
    bool_expr context::mk_ge(real_expr lhs, real_expr rhs) { return utils::make_s_ptr<real_ge>(*this, lhs, rhs); }
    bool_expr context::mk_gt(real_expr lhs, real_expr rhs) { return utils::make_s_ptr<real_gt>(*this, lhs, rhs); }

    bool_expr context::to_cnf(bool_expr expr) { return distribute(push_negations(expr)); }

    bool_expr context::push_negations(bool_expr expr)
    {
        if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
        {
            if (auto not_xpr_xpr = utils::s_ptr_cast<not_expr>(not_xpr->arg()))
                return push_negations(not_xpr_xpr->arg());
            else if (auto and_xpr = utils::s_ptr_cast<and_expr>(not_xpr->arg()))
            {
                std::vector<bool_expr> args;
                for (const auto &arg : and_xpr->args())
                    args.push_back(push_negations(mk_not(arg)));
                return mk_or(std::move(args));
            }
            else if (auto or_xpr = utils::s_ptr_cast<or_expr>(not_xpr->arg()))
            {
                std::vector<bool_expr> args;
                for (const auto &arg : or_xpr->args())
                    args.push_back(push_negations(mk_not(arg)));
                return mk_and(std::move(args));
            }
            else
                return expr;
        }
        else if (auto and_xpr = utils::s_ptr_cast<and_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : and_xpr->args())
                args.push_back(push_negations(arg));
            return mk_and(std::move(args));
        }
        else if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : or_xpr->args())
                args.push_back(push_negations(arg));
            return mk_or(std::move(args));
        }
        else
            return expr;
    }

    bool_expr context::distribute(bool_expr expr)
    {
        if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : or_xpr->args())
                args.push_back(distribute(arg));
            std::vector<bool_expr> new_args;
            for (const auto &arg : args)
                if (auto and_xpr = utils::s_ptr_cast<and_expr>(arg))
                    for (const auto &and_arg : and_xpr->args())
                        new_args.push_back(and_arg);
                else
                    new_args.push_back(arg);
            return mk_or(std::move(new_args));
        }
        else if (auto and_xpr = utils::s_ptr_cast<and_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : and_xpr->args())
                args.push_back(distribute(arg));
            return mk_and(std::move(args));
        }
        else
            return expr;
    }

    bool_expr context::simplify(bool_expr expr)
    {
        if (auto int_lt_xpr = utils::s_ptr_cast<int_lt>(expr))
        {
            std::vector<int_expr> lhs;
            utils::integer rhs;
            auto lin = linearize(int_lt_xpr->left() - int_lt_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<int_expr>{mk_int_var(name), mk_int_const(coeff)}));
                }
            return mk_lt(mk_sum(std::move(lhs)), mk_int_const(rhs));
        }
        else if (auto int_le_xpr = utils::s_ptr_cast<int_le>(expr))
        {
            std::vector<int_expr> lhs;
            utils::integer rhs;
            auto lin = linearize(int_le_xpr->left() - int_le_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<int_expr>{mk_int_var(name), mk_int_const(coeff)}));
                }
            return mk_le(mk_sum(std::move(lhs)), mk_int_const(rhs));
        }
        else if (auto int_eq_xpr = utils::s_ptr_cast<int_eq>(expr))
        {
            std::vector<int_expr> lhs;
            utils::integer rhs;
            auto lin = linearize(int_eq_xpr->left() - int_eq_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<int_expr>{mk_int_var(name), mk_int_const(coeff)}));
                }
            return mk_eq(mk_sum(std::move(lhs)), mk_int_const(rhs));
        }
        else if (auto int_ge_xpr = utils::s_ptr_cast<int_ge>(expr))
        {
            std::vector<int_expr> lhs;
            utils::integer rhs;
            auto lin = linearize(int_ge_xpr->left() - int_ge_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<int_expr>{mk_int_var(name), mk_int_const(coeff)}));
                }
            return mk_ge(mk_sum(std::move(lhs)), mk_int_const(rhs));
        }
        else if (auto int_gt_xpr = utils::s_ptr_cast<int_gt>(expr))
        {
            std::vector<int_expr> lhs;
            utils::integer rhs;
            auto lin = linearize(int_gt_xpr->left() - int_gt_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<int_expr>{mk_int_var(name), mk_int_const(coeff)}));
                }
            return mk_gt(mk_sum(std::move(lhs)), mk_int_const(rhs));
        }
        else if (auto real_lt_xpr = utils::s_ptr_cast<real_lt>(expr))
        {
            std::vector<real_expr> lhs;
            utils::rational rhs;
            auto lin = linearize(real_lt_xpr->left() - real_lt_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<real_expr>{mk_real_var(name), mk_real_const(coeff)}));
                }
            return mk_lt(mk_sum(std::move(lhs)), mk_real_const(rhs));
        }
        else if (auto real_le_xpr = utils::s_ptr_cast<real_le>(expr))
        {
            std::vector<real_expr> lhs;
            utils::rational rhs;
            auto lin = linearize(real_le_xpr->left() - real_le_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<real_expr>{mk_real_var(name), mk_real_const(coeff)}));
                }
            return mk_le(mk_sum(std::move(lhs)), mk_real_const(rhs));
        }
        else if (auto real_eq_xpr = utils::s_ptr_cast<real_eq>(expr))
        {
            std::vector<real_expr> lhs;
            utils::rational rhs;
            auto lin = linearize(real_eq_xpr->left() - real_eq_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<real_expr>{mk_real_var(name), mk_real_const(coeff)}));
                }
            return mk_eq(mk_sum(std::move(lhs)), mk_real_const(rhs));
        }
        else if (auto real_ge_xpr = utils::s_ptr_cast<real_ge>(expr))
        {
            std::vector<real_expr> lhs;
            utils::rational rhs;
            auto lin = linearize(real_ge_xpr->left() - real_ge_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<real_expr>{mk_real_var(name), mk_real_const(coeff)}));
                }
            return mk_ge(mk_sum(std::move(lhs)), mk_real_const(rhs));
        }
        else if (auto real_gt_xpr = utils::s_ptr_cast<real_gt>(expr))
        {
            std::vector<real_expr> lhs;
            utils::rational rhs;
            auto lin = linearize(real_gt_xpr->left() - real_gt_xpr->right());
            for (const auto &[name, coeff] : lin)
                if (!is_zero(coeff))
                {
                    if (name.empty())
                        rhs -= coeff;
                    else
                        lhs.push_back(mk_mul(std::vector<real_expr>{mk_real_var(name), mk_real_const(coeff)}));
                }
            return mk_gt(mk_sum(std::move(lhs)), mk_real_const(rhs));
        }
        else
            throw std::runtime_error("Unknown integer expression type.");
    }

    std::map<std::string, utils::integer> context::linearize(int_expr expr)
    {
        if (auto c_xpr = utils::s_ptr_cast<int_const>(expr))
            return {{"", c_xpr->val()}}; // we have a constant..
        else if (auto var_xpr = utils::s_ptr_cast<int_var>(expr))
            return {{var_xpr->get_name(), utils::integer::one}}; // we have a variable..

        std::map<std::string, utils::integer> dist;
        if (auto sum_xpr = utils::s_ptr_cast<int_sum>(expr))
        {
            for (const auto &arg : sum_xpr->args())
            {
                auto arg_dist = linearize(arg);
                for (const auto &[name, coeff] : arg_dist)
                    dist[name] += coeff;
            }
        }
        else if (auto sub_xpr = utils::s_ptr_cast<int_sub>(expr))
        {
            auto arg_dist = linearize(sub_xpr->args()[0]);
            for (const auto &[name, coeff] : arg_dist)
                dist[name] += coeff;
            for (size_t i = 1; i < sub_xpr->args().size(); ++i)
            {
                arg_dist = linearize(sub_xpr->args()[i]);
                for (const auto &[name, coeff] : arg_dist)
                    dist[name] -= coeff;
            }
        }
        else if (auto mul_xpr = utils::s_ptr_cast<int_mul>(expr))
        {
            dist = {{"", utils::integer::one}};
            for (const auto &arg : mul_xpr->args())
            {
                auto arg_dist = linearize(arg);
                std::map<std::string, utils::integer> new_dist;
                for (const auto &[name, coeff] : dist)
                    for (const auto &[arg_name, arg_coeff] : arg_dist)
                        new_dist[name + arg_name] += coeff * arg_coeff;
                dist = std::move(new_dist);
            }
        }
        else if (auto div_xpr = utils::s_ptr_cast<int_div>(expr))
        {
            dist = {{"", utils::integer::one}};
            auto arg_dist = linearize(div_xpr->args()[0]);
            std::map<std::string, utils::integer> new_dist;
            for (const auto &[name, coeff] : dist)
                for (const auto &[arg_name, arg_coeff] : arg_dist)
                    new_dist[name + arg_name] += coeff * arg_coeff;
            dist = std::move(new_dist);
            for (size_t i = 1; i < div_xpr->args().size(); ++i)
            {
                arg_dist = linearize(div_xpr->args()[i]);
                std::map<std::string, utils::integer> new_dist;
                for (const auto &[name, coeff] : dist)
                    for (const auto &[arg_name, arg_coeff] : arg_dist)
                        new_dist[name + arg_name] += coeff / arg_coeff;
                dist = std::move(new_dist);
            }
        }
        else
            throw std::runtime_error("Unknown real expression type.");
        return dist;
    }

    std::map<std::string, utils::rational> context::linearize(real_expr expr)
    {
        if (auto c_xpr = utils::s_ptr_cast<real_const>(expr))
            return {{"", c_xpr->val()}}; // we have a constant..
        else if (auto var_xpr = utils::s_ptr_cast<real_var>(expr))
            return {{var_xpr->get_name(), utils::rational::one}}; // we have a variable..

        std::map<std::string, utils::rational> dist;
        if (auto sum_xpr = utils::s_ptr_cast<real_sum>(expr))
        {
            for (const auto &arg : sum_xpr->args())
            {
                auto arg_dist = linearize(arg);
                for (const auto &[name, coeff] : arg_dist)
                    dist[name] += coeff;
            }
        }
        else if (auto sub_xpr = utils::s_ptr_cast<real_sub>(expr))
        {
            auto arg_dist = linearize(sub_xpr->args()[0]);
            for (const auto &[name, coeff] : arg_dist)
                dist[name] += coeff;
            for (size_t i = 1; i < sub_xpr->args().size(); ++i)
            {
                arg_dist = linearize(sub_xpr->args()[i]);
                for (const auto &[name, coeff] : arg_dist)
                    dist[name] -= coeff;
            }
        }
        else if (auto mul_xpr = utils::s_ptr_cast<real_mul>(expr))
        {
            dist = {{"", utils::rational::one}};
            for (const auto &arg : mul_xpr->args())
            {
                auto arg_dist = linearize(arg);
                std::map<std::string, utils::rational> new_dist;
                for (const auto &[name, coeff] : dist)
                    for (const auto &[arg_name, arg_coeff] : arg_dist)
                        new_dist[name + arg_name] += coeff * arg_coeff;
                dist = std::move(new_dist);
            }
        }
        else if (auto div_xpr = utils::s_ptr_cast<real_div>(expr))
        {
            dist = {{"", utils::rational::one}};
            auto arg_dist = linearize(div_xpr->args()[0]);
            std::map<std::string, utils::rational> new_dist;
            for (const auto &[name, coeff] : dist)
                for (const auto &[arg_name, arg_coeff] : arg_dist)
                    new_dist[name + arg_name] += coeff * arg_coeff;
            dist = std::move(new_dist);
            for (size_t i = 1; i < div_xpr->args().size(); ++i)
            {
                arg_dist = linearize(div_xpr->args()[i]);
                std::map<std::string, utils::rational> new_dist;
                for (const auto &[name, coeff] : dist)
                    for (const auto &[arg_name, arg_coeff] : arg_dist)
                        new_dist[name + arg_name] += coeff / arg_coeff;
                dist = std::move(new_dist);
            }
        }
        else
            throw std::runtime_error("Unknown real expression type.");
        return dist;
    }

    bool_expr operator&&(bool_expr lhs, bool_expr rhs) noexcept { return lhs->get_ctx().mk_and({lhs, rhs}); }
    bool_expr operator&&(bool_expr lhs, bool rhs) noexcept { return lhs->get_ctx().mk_and({lhs, lhs->get_ctx().mk_bool_const(rhs)}); }
    bool_expr operator&&(bool lhs, bool_expr rhs) noexcept { return rhs->get_ctx().mk_and({rhs->get_ctx().mk_bool_const(lhs), rhs}); }
    bool_expr operator||(bool_expr lhs, bool_expr rhs) noexcept { return lhs->get_ctx().mk_or({lhs, rhs}); }
    bool_expr operator||(bool_expr lhs, bool rhs) noexcept { return lhs->get_ctx().mk_or({lhs, lhs->get_ctx().mk_bool_const(rhs)}); }
    bool_expr operator||(bool lhs, bool_expr rhs) noexcept { return rhs->get_ctx().mk_or({rhs->get_ctx().mk_bool_const(lhs), rhs}); }
    bool_expr operator!(bool_expr arg) noexcept { return arg->get_ctx().mk_not(arg); }

    int_expr operator+(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<int_expr>{lhs, rhs}); }
    int_expr operator+(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(rhs)}); }
    int_expr operator+(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_sum(std::vector<int_expr>{rhs->get_ctx().mk_int_const(lhs), rhs}); }
    int_expr operator+(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))}); }
    int_expr operator+(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_sum(std::vector<int_expr>{rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs}); }

    int_expr operator-(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<int_expr>{lhs, rhs}); }
    int_expr operator-(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(rhs)}); }
    int_expr operator-(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_sub(std::vector<int_expr>{rhs->get_ctx().mk_int_const(lhs), rhs}); }
    int_expr operator-(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))}); }
    int_expr operator-(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_sub(std::vector<int_expr>{rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs}); }

    int_expr operator*(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<int_expr>{lhs, rhs}); }
    int_expr operator*(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(rhs)}); }
    int_expr operator*(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_mul(std::vector<int_expr>{rhs->get_ctx().mk_int_const(lhs), rhs}); }
    int_expr operator*(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))}); }
    int_expr operator*(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_mul(std::vector<int_expr>{rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs}); }

    int_expr operator/(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<int_expr>{lhs, rhs}); }
    int_expr operator/(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(rhs)}); }
    int_expr operator/(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_div(std::vector<int_expr>{rhs->get_ctx().mk_int_const(lhs), rhs}); }
    int_expr operator/(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<int_expr>{lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))}); }
    int_expr operator/(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_div(std::vector<int_expr>{rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs}); }

    bool_expr operator<(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, rhs); }
    bool_expr operator<(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, lhs->get_ctx().mk_int_const(rhs)); }
    bool_expr operator<(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_lt(rhs->get_ctx().mk_int_const(lhs), rhs); }
    bool_expr operator<(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))); }
    bool_expr operator<(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_lt(rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs); }
    bool_expr operator<=(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_le(lhs, rhs); }
    bool_expr operator<=(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_le(lhs, lhs->get_ctx().mk_int_const(rhs)); }
    bool_expr operator<=(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_le(rhs->get_ctx().mk_int_const(lhs), rhs); }
    bool_expr operator<=(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_le(lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))); }
    bool_expr operator<=(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_le(rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs); }
    bool_expr operator==(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, rhs); }
    bool_expr operator==(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, lhs->get_ctx().mk_int_const(rhs)); }
    bool_expr operator==(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_eq(rhs->get_ctx().mk_int_const(lhs), rhs); }
    bool_expr operator==(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))); }
    bool_expr operator==(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_eq(rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs); }
    bool_expr operator>=(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, rhs); }
    bool_expr operator>=(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, lhs->get_ctx().mk_int_const(rhs)); }
    bool_expr operator>=(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_ge(rhs->get_ctx().mk_int_const(lhs), rhs); }
    bool_expr operator>=(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))); }
    bool_expr operator>=(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_ge(rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs); }
    bool_expr operator>(int_expr lhs, int_expr rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, rhs); }
    bool_expr operator>(int_expr lhs, utils::integer rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, lhs->get_ctx().mk_int_const(rhs)); }
    bool_expr operator>(utils::integer lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_gt(rhs->get_ctx().mk_int_const(lhs), rhs); }
    bool_expr operator>(int_expr lhs, int rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, lhs->get_ctx().mk_int_const(utils::integer(rhs))); }
    bool_expr operator>(int lhs, int_expr rhs) noexcept { return rhs->get_ctx().mk_gt(rhs->get_ctx().mk_int_const(utils::integer(lhs)), rhs); }

    real_expr operator+(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<real_expr>{lhs, rhs}); }
    real_expr operator+(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(rhs)}); }
    real_expr operator+(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_sum(std::vector<real_expr>{rhs->get_ctx().mk_real_const(lhs), rhs}); }
    real_expr operator+(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_sum(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))}); }
    real_expr operator+(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_sum(std::vector<real_expr>{rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs}); }

    real_expr operator-(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<real_expr>{lhs, rhs}); }
    real_expr operator-(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(rhs)}); }
    real_expr operator-(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_sub(std::vector<real_expr>{rhs->get_ctx().mk_real_const(lhs), rhs}); }
    real_expr operator-(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_sub(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))}); }
    real_expr operator-(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_sub(std::vector<real_expr>{rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs}); }

    real_expr operator*(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<real_expr>{lhs, rhs}); }
    real_expr operator*(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(rhs)}); }
    real_expr operator*(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_mul(std::vector<real_expr>{rhs->get_ctx().mk_real_const(lhs), rhs}); }
    real_expr operator*(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_mul(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))}); }
    real_expr operator*(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_mul(std::vector<real_expr>{rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs}); }

    real_expr operator/(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<real_expr>{lhs, rhs}); }
    real_expr operator/(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(rhs)}); }
    real_expr operator/(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_div(std::vector<real_expr>{rhs->get_ctx().mk_real_const(lhs), rhs}); }
    real_expr operator/(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_div(std::vector<real_expr>{lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))}); }
    real_expr operator/(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_div(std::vector<real_expr>{rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs}); }

    bool_expr operator<(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, rhs); }
    bool_expr operator<(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, lhs->get_ctx().mk_real_const(rhs)); }
    bool_expr operator<(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_lt(rhs->get_ctx().mk_real_const(lhs), rhs); }
    bool_expr operator<(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_lt(lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))); }
    bool_expr operator<(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_lt(rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs); }
    bool_expr operator<=(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_le(lhs, rhs); }
    bool_expr operator<=(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_le(lhs, lhs->get_ctx().mk_real_const(rhs)); }
    bool_expr operator<=(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_le(rhs->get_ctx().mk_real_const(lhs), rhs); }
    bool_expr operator<=(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_le(lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))); }
    bool_expr operator<=(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_le(rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs); }
    bool_expr operator==(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, rhs); }
    bool_expr operator==(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, lhs->get_ctx().mk_real_const(rhs)); }
    bool_expr operator==(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_eq(rhs->get_ctx().mk_real_const(lhs), rhs); }
    bool_expr operator==(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_eq(lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))); }
    bool_expr operator==(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_eq(rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs); }
    bool_expr operator>=(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, rhs); }
    bool_expr operator>=(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, lhs->get_ctx().mk_real_const(rhs)); }
    bool_expr operator>=(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_ge(rhs->get_ctx().mk_real_const(lhs), rhs); }
    bool_expr operator>=(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_ge(lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))); }
    bool_expr operator>=(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_ge(rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs); }
    bool_expr operator>(real_expr lhs, real_expr rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, rhs); }
    bool_expr operator>(real_expr lhs, utils::rational rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, lhs->get_ctx().mk_real_const(rhs)); }
    bool_expr operator>(utils::rational lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_gt(rhs->get_ctx().mk_real_const(lhs), rhs); }
    bool_expr operator>(real_expr lhs, double rhs) noexcept { return lhs->get_ctx().mk_gt(lhs, lhs->get_ctx().mk_real_const(utils::rational(rhs))); }
    bool_expr operator>(double lhs, real_expr rhs) noexcept { return rhs->get_ctx().mk_gt(rhs->get_ctx().mk_real_const(utils::rational(lhs)), rhs); }
} // namespace semitone
