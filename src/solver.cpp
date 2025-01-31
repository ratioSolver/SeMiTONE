#include "solver.hpp"
#include "lit.hpp"
#include "logging.hpp"
#include <algorithm>

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
                if (auto and_xpr = utils::s_ptr_cast<and_expr>(arg))
                    for (const auto &and_arg : and_xpr->args())
                        new_args.push_back(and_arg);
                else
                    new_args.push_back(arg);
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
            assigns.push_back(utils::Undefined);
            watches.emplace_back();
            watches.emplace_back();
            level.emplace_back(0);
            reason.emplace_back(std::nullopt);
            return id;
        }
    }

    void solver::add_clause(bool_expr expr)
    {
        std::vector<utils::lit> lits;
        if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
        {
            for (const auto &arg : or_xpr->args())
            {
                if (auto not_xpr = utils::s_ptr_cast<not_expr>(arg))
                    lits.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
                else
                    lits.push_back(utils::lit(add_var(arg->get_name()), true));
            }
        }
        else if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
            lits.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
        else
            lits.push_back(utils::lit(add_var(expr->get_name()), true));

        // we sort the clause to make sure that we can easily check for duplicates..
        std::sort(lits.begin(), lits.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        utils::lit p;
        size_t j = 0;
        for (auto it = lits.cbegin(); it != lits.cend(); ++it)
            if (value(*it) == utils::True || *it == !p)
                return; // the clause is already satisfied or represents a tautology..
            else if (value(*it) != utils::False && *it != p)
            { // we need to include this literal in the clause..
                p = *it;
                lits[j++] = p;
            }
        lits.resize(j);

        switch (lits.size())
        {
        case 0: // the clause is unsatisfable under the current assignment (so is the problem)..
            throw unsolvable_exception();
        case 1: // the clause is unique under the current assignment..
            if (!enqueue(lits[0]))
                throw unsolvable_exception();
            break;
        default: // we need to create a new clause..
            clauses.push_back(new clause(*this, std::move(lits)));
        }
    }

    bool solver::enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c) noexcept
    {
        if (auto val = value(p); val != utils::Undefined)
            return val; // the literal is already assigned..
        assigns[variable(p)] = sign(p);
        level[variable(p)] = decision_level();
        if (c)
            reason[variable(p)] = c;
        trail.push_back(p);
        prop_queue.push(p);
        return true;
    }

    clause::clause(solver &slv, std::vector<utils::lit> &&lits) noexcept : slv(slv), lits(std::move(lits)) {}
} // namespace semitone
