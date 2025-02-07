#include "la_theory.hpp"
#include "network.hpp"
#include <cassert>

namespace semitone
{
    la_theory::la_theory(network &net) noexcept : theory(net) {}

    utils::var la_theory::new_int(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int.push_back(true);
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_real(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int.push_back(false);
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_slack(utils::lin &&xpr) noexcept
    {
        auto var = vals.size();
        is_int.push_back(false);

        utils::inf_rational val(xpr.known_term), lb(xpr.known_term), ub(xpr.known_term);
        std::vector<utils::lit> lb_reason, ub_reason;
        for (const auto &[v, c] : xpr.vars)
        {
            val += c * vals[v];
            lb += (is_positive(c) ? c_bounds[lb_index(v)].value : c_bounds[ub_index(v)].value) * c;
            lb_reason.insert(lb_reason.end(), c_bounds[lb_index(v)].reason.cbegin(), c_bounds[lb_index(v)].reason.cend());
            ub += (is_positive(c) ? c_bounds[ub_index(v)].value : c_bounds[lb_index(v)].value) * c;
            ub_reason.insert(ub_reason.end(), c_bounds[ub_index(v)].reason.cbegin(), c_bounds[ub_index(v)].reason.cend());
        }
        c_bounds.emplace_back(bound{lb, std::move(lb_reason)});
        c_bounds.emplace_back(bound{ub, std::move(ub_reason)});
        vals.push_back(val);
        a_watches.emplace_back();
        t_watches.emplace_back();

        new_row(var, std::move(xpr));

        return var;
    }

    void la_theory::add_lt(utils::lin &lhs, utils::lin &rhs, bool strict)
    {
        utils::lin expr = lhs - rhs;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<utils::var> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->l;
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            if (strict && expr.known_term >= 0)
                throw unsolvable_exception(); // the problem is unsatisfiable..
            else if (expr.known_term > 0)
                throw unsolvable_exception(); // the problem is unsatisfiable..
            return;                           // the constraint is already satisfied..
        case 1:
        { // the expression is a single variable..
            const auto [v, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
            if (c > 0)
            { // `v` <= `c_right`..
                if (ub(v) <= c_right)
                    return; // the constraint is already satisfied..
                else if (lb(v) > c_right)
                    throw unsolvable_exception(); // the problem is unsatisfiable..
                // we update the upper bound of `v`..
                c_bounds[ub_index(v)].value = c_right;
            }
            else
            { // `v` >= `c_right`..
                if (lb(v) >= c_right)
                    return; // the constraint is already satisfied..
                else if (ub(v) < c_right)
                    throw unsolvable_exception(); // the problem is unsatisfiable..
                // we update the lower bound of `v`..
                c_bounds[lb_index(v)].value = c_right;
            }
        }
        break;
        default:
        { // the expression is an inequality with multiple variables
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0);
            expr.known_term = utils::rational::zero;

            if (ub(expr) <= c_right)
                return; // the constraint is already satisfied..
            else if (lb(expr) > c_right)
                throw unsolvable_exception(); // the problem is unsatisfiable..

            // we add a slack variable to the tableau..
            auto slack = new_slack(std::move(expr));
            // .. and update its upper bound..
            c_bounds[ub_index(slack)].value = c_right;
        }
        }
    }

    void la_theory::new_lt(utils::lit &p, utils::lin &lhs, utils::lin &rhs, bool strict)
    {
        utils::lin expr = lhs - rhs;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<utils::var> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->l;
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            if (strict && expr.known_term >= 0)
                net.add_clause({!p});
            else if (expr.known_term > 0)
                net.add_clause({!p});
            return; // the constraint is already satisfied..
        case 1:
        { // the expression is a single variable..
            const auto [v, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
            if (c > 0)
            { // `v` <= `c_right`..
                if (ub(v) <= c_right)
                    return; // the constraint is already satisfied..
                else if (lb(v) > c_right)
                    net.add_clause({!p});
                v_asrts.emplace(variable(p), new la_assertion(p, v, op::leq, c_right));
                bind(variable(p)); // we get notified when the variable `v` changes..
            }
            else
            { // `v` >= `c_right`..
                if (lb(v) >= c_right)
                    return; // the constraint is already satisfied..
                else if (ub(v) < c_right)
                    net.add_clause({!p});
                v_asrts.emplace(variable(p), new la_assertion(p, v, op::geq, c_right));
                bind(variable(p)); // we get notified when the variable `v` changes..
            }
        }
        break;
        default:
        { // the expression is an inequality with multiple variables
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0);
            expr.known_term = utils::rational::zero;

            if (ub(expr) <= c_right)
                return; // the constraint is already satisfied..
            else if (lb(expr) > c_right)
                net.add_clause({!p});

            // we add a slack variable to the tableau..
            auto slack = new_slack(std::move(expr));
            // .. and update its upper bound..
            c_bounds[ub_index(slack)].value = c_right;
            v_asrts.emplace(variable(p), new la_assertion(p, slack, op::leq, c_right));
            bind(variable(p)); // we get notified when the slack variable changes..
        }
        }
    }

    bool la_theory::propagate(const utils::lit &p) noexcept { return true; }

    bool la_theory::check() noexcept { return true; }

    void la_theory::push() noexcept {}

    void la_theory::pop() noexcept {}

    void la_theory::new_row(const utils::var x_i, utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, new la_eq(x_i, std::move(xpr)));
    }
} // namespace semitone