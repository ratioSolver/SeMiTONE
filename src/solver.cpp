#include "solver.hpp"
#include "lit.hpp"
#include "logging.hpp"

namespace semitone
{
    solver::solver(context &ctx) : ctx(ctx) {}

    void solver::add(bool_expr expr)
    {
        auto cnf_expr = to_cnf(expr); // Convert to CNF
        if (auto and_xpr = utils::s_ptr_cast<and_expr>(cnf_expr))
            for (const auto &arg : and_xpr->args())
                add_clause(arg);
        else
            add_clause(cnf_expr);
    }

    bool_expr solver::to_cnf(bool_expr expr) { return distribute(push_negations(expr)); }

    bool_expr solver::push_negations(bool_expr expr)
    {
        if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
        {
            if (auto not_xpr_xpr = utils::s_ptr_cast<not_expr>(not_xpr->arg()))
            {
                return push_negations(not_xpr_xpr->arg());
            }
            else if (auto and_xpr = utils::s_ptr_cast<and_expr>(not_xpr->arg()))
            {
                std::vector<bool_expr> args;
                for (const auto &arg : and_xpr->args())
                    args.push_back(push_negations(ctx.mk_not(arg)));
                return ctx.mk_or(std::move(args));
            }
            else if (auto or_xpr = utils::s_ptr_cast<or_expr>(not_xpr->arg()))
            {
                std::vector<bool_expr> args;
                for (const auto &arg : or_xpr->args())
                    args.push_back(push_negations(ctx.mk_not(arg)));
                return ctx.mk_and(std::move(args));
            }
            else
                return expr;
        }
        else if (auto and_xpr = utils::s_ptr_cast<and_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : and_xpr->args())
                args.push_back(push_negations(arg));
            return ctx.mk_and(std::move(args));
        }
        else if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : or_xpr->args())
                args.push_back(push_negations(arg));
            return ctx.mk_or(std::move(args));
        }
        else
            return expr;
    }

    bool_expr solver::distribute(bool_expr expr)
    {
        if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : or_xpr->args())
                args.push_back(distribute(arg));
            std::vector<bool_expr> new_args;
            for (const auto &arg : args)
            {
                if (auto and_xpr = utils::s_ptr_cast<and_expr>(arg))
                {
                    for (const auto &and_arg : and_xpr->args())
                        new_args.push_back(and_arg);
                }
                else
                {
                    new_args.push_back(arg);
                }
            }
            return ctx.mk_or(std::move(new_args));
        }
        else if (auto and_xpr = utils::s_ptr_cast<and_expr>(expr))
        {
            std::vector<bool_expr> args;
            for (const auto &arg : and_xpr->args())
                args.push_back(distribute(arg));
            return ctx.mk_and(std::move(args));
        }
        else
            return expr;
    }

    size_t solver::add_var(std::string_view name)
    {
        if (auto it = var_map.find(name.data()); it != var_map.end())
            return it->second;
        else
        {
            size_t id = var_map.size();
            var_map.emplace(name.data(), id);
            return id;
        }
    }

    void solver::add_clause(bool_expr expr)
    {
        std::vector<utils::lit> clause;
        if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            for (const auto &arg : or_xpr->args())
            {
                if (auto not_xpr = utils::s_ptr_cast<not_expr>(arg))
                    clause.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
                else
                    clause.push_back(utils::lit(add_var(arg->get_name()), true));
            }
        }
        else if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
            clause.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
        else
            clause.push_back(utils::lit(add_var(expr->get_name()), true));
    }
} // namespace semitone
