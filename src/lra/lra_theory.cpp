#include <cassert>
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "lra_assertion.hpp"
#include "lra_eq.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_theory::lra_theory(std::shared_ptr<sat_core> sat) noexcept : theory(sat) {}

    VARIABLE_TYPE lra_theory::new_var() noexcept
    {
        auto var = vals.size();
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::negative_infinite), utils::TRUE_lit});
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::positive_infinite), utils::TRUE_lit});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }
    VARIABLE_TYPE lra_theory::new_var(const utils::lin &&l) noexcept
    {
        assert(sat->root_level());
        const auto slack = new_var();
        c_bounds[lb_index(slack)] = {lb(l), utils::TRUE_lit}; // we set the lower bound of the slack variable to the lower bound of the linear expression
        c_bounds[ub_index(slack)] = {ub(l), utils::TRUE_lit}; // we set the upper bound of the slack variable to the upper bound of the linear expression
        vals[slack] = value(l);                               // we set the value of the slack variable to the value of the linear expression
        new_row(slack, std::move(l));                         // we add the new row `slack = ...` to the tableau
        return slack;
    }

    utils::lit lra_theory::new_lt(const utils::lin &left, const utils::lin &right) noexcept
    {
        // x+3<y+4 -> x-y<=1-ε
        utils::lin expr = left - right;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<VARIABLE_TYPE> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->get_lin();
            }

        const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, -1);
        expr.known_term = utils::rational::zero;

        if (ub(expr) <= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (lb(expr) > c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        // we create a new control variable..
        const auto ctr = sat->new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        v_asrts.emplace(ctr, std::make_unique<lra_leq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }
    utils::lit lra_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
        // x+3<=y+4 -> x-y<=1
        utils::lin expr = left - right;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<VARIABLE_TYPE> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->get_lin();
            }

        const utils::inf_rational c_right = utils::inf_rational(-expr.known_term);
        expr.known_term = utils::rational::zero;

        if (ub(expr) <= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (lb(expr) > c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        // we create a new control variable..
        const auto ctr = sat->new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        v_asrts.emplace(ctr, std::make_unique<lra_leq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }
    utils::lit lra_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept { return sat->new_conj({new_leq(left, right), new_geq(left, right)}); }
    utils::lit lra_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
    {
        // x+3>=y+4 -> x-y>=1
        utils::lin expr = left - right;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<VARIABLE_TYPE> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->get_lin();
            }

        const utils::inf_rational c_right = utils::inf_rational(-expr.known_term);
        expr.known_term = utils::rational::zero;

        if (lb(expr) >= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (ub(expr) < c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        // we create a new control variable..
        const auto ctr = sat->new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        v_asrts.emplace(ctr, std::make_unique<lra_geq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }
    utils::lit lra_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
    {
        // x+3>y+4 -> x-y>=1+ε
        utils::lin expr = left - right;
        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau
        std::vector<VARIABLE_TYPE> vars;
        vars.reserve(expr.vars.size());
        for ([[maybe_unused]] const auto &[v, c] : expr.vars)
            vars.push_back(v);
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = expr.vars.at(v);
                expr.vars.erase(v);
                expr += c * tableau.at(v)->get_lin();
            }

        const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, 1);
        expr.known_term = utils::rational::zero;

        if (lb(expr) >= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (ub(expr) < c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        // we create a new control variable..
        const auto ctr = sat->new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        v_asrts.emplace(ctr, std::make_unique<lra_geq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }

    void lra_theory::new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend());
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, std::make_unique<lra_eq>(*this, x_i, std::move(xpr)));
    }
} // namespace semitone