#include "la_theory.hpp"
#include "network.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cassert>

#ifdef BUILD_LISTENERS
#define FIRE_ON_CHANGE(v)                                    \
    if (auto it = listeners.find(v); it != listeners.cend()) \
        for (const auto &l : it->second)                     \
            l->on_arith_change(v);
#else
#define FIRE_ON_CHANGE(v)
#endif

namespace semitone
{
    la_theory::la_theory(network &net) noexcept : theory(net) {}

    utils::var la_theory::new_int(const utils::rational &lb, const utils::rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int_var.push_back(true);
        c_bounds.emplace_back(bound{utils::inf_rational(lb), {}});
        c_bounds.emplace_back(bound{utils::inf_rational(ub), {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_int(utils::lin &&xpr) noexcept
    {
        auto var = vals.size();
        is_int_var.push_back(true);

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

    utils::var la_theory::new_real(const utils::rational &lb, const utils::rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int_var.push_back(false);
        c_bounds.emplace_back(bound{utils::inf_rational(lb), {}});
        c_bounds.emplace_back(bound{utils::inf_rational(ub), {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_real(utils::lin &&xpr) noexcept
    {
        auto var = vals.size();
        is_int_var.push_back(false);

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

    void la_theory::new_lt(const utils::lin &lhs, const utils::lin &rhs, const utils::lit &p, bool strict)
    {
        assert(net.value(p) != utils::False);
        utils::lin expr = lhs - rhs;

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant..
            if (strict && expr.known_term >= 0)
                return net.new_clause({!p}); // the constraint is conflicting..
            else if (expr.known_term > 0)
                return net.new_clause({!p}); // the constraint is conflicting..
            return;                          // the constraint is already satisfied..
        case 1:
        { // the expression is a single variable..
            const auto [v, c] = *expr.vars.cbegin();
            assert(c != 0);
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
            if (c > 0)
            { // `v` <= `c_right`..
                if (ub(v) <= (is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right))
                    return; // the constraint is already satisfied..
                else if (lb(v) > (is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right))
                    return net.new_clause({!p}); // the constraint is conflicting..
                if (net.value(p) == utils::True)
                { // we update the upper bound..
                    if (!assert_upper(v, is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right, {}))
                        throw unsolvable_exception();
                }
                else
                { // we add the assertion to the list of assertions..
                    LOG_TRACE("[" << to_string(p) << "] x" << std::to_string(v) << " <= " << to_string(is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right));
                    v_asrts[variable(p)].emplace(new la_assertion(p, v, op::leq, is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right));
                    bind(variable(p)); // we get notified when the variable `v` changes..
                }
            }
            else
            { // `v` >= `c_right`..
                if (lb(v) >= (is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right))
                    return; // the constraint is already satisfied..
                else if (ub(v) < (is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right))
                    return net.new_clause({!p}); // the constraint is conflicting..
                if (net.value(p) == utils::True)
                { // we update the lower bound..
                    if (!assert_lower(v, is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right, {}))
                        throw unsolvable_exception();
                }
                else
                { // we add the assertion to the list of assertions..
                    LOG_TRACE("[" << to_string(p) << "] x" << std::to_string(v) << " >= " << to_string(is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right));
                    v_asrts[variable(p)].emplace(new la_assertion(p, v, op::geq, is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right));
                    bind(variable(p)); // we get notified when the variable `v` changes..
                }
            }
            return;
        }
        default:
        {
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
                    return net.new_clause({!p}); // the constraint is conflicting..
                else if (expr.known_term > 0)
                    return net.new_clause({!p}); // the constraint is conflicting..
                return;                          // the constraint is already satisfied..
            case 1:
            { // the expression is a single variable..
                const auto [v, c] = *expr.vars.cbegin();
                assert(c != 0);
                const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, strict ? -1 : 0) / c; // the right-hand side of the constraint is the division of the negation of the known term minus an infinitesimal by the coefficient..
                if (c > 0)
                { // `v` <= `c_right`..
                    if (ub(v) <= (is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right))
                        return; // the constraint is already satisfied..
                    else if (lb(v) > (is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right))
                        return net.new_clause({!p}); // the constraint is conflicting..
                    if (net.value(p) == utils::True)
                    { // we update the upper bound..
                        if (!assert_upper(v, is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right, {}))
                            throw unsolvable_exception();
                    }
                    else
                    { // we add the assertion to the list of assertions..
                        LOG_TRACE("[ " << to_string(p) << " ] x" << std::to_string(v) << " <= " << to_string(is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right));
                        v_asrts[variable(p)].emplace(new la_assertion(p, v, op::leq, is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right));
                        bind(variable(p)); // we get notified when the variable `v` changes..
                    }
                }
                else
                { // `v` >= `c_right`..
                    if (lb(v) >= (is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right))
                        return; // the constraint is already satisfied..
                    else if (ub(v) < (is_int(v) ? utils::inf_rational(floor(c_right.get_rational())) : c_right))
                        return net.new_clause({!p}); // the constraint is conflicting..
                    if (net.value(p) == utils::True)
                    { // we update the lower bound..
                        if (!assert_lower(v, is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right, {}))
                            throw unsolvable_exception();
                    }
                    else
                    { // we add the assertion to the list of assertions..
                        LOG_TRACE("[ " << to_string(p) << " ] x" << std::to_string(v) << " >= " << to_string(is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right));
                        v_asrts[variable(p)].emplace(new la_assertion(p, v, op::geq, is_int(v) ? utils::inf_rational(ceil(c_right.get_rational())) : c_right));
                        bind(variable(p)); // we get notified when the variable `v` changes..
                    }
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
                    net.new_clause({!p});

                // we add a slack variable to the tableau..
                auto slack = new_real(std::move(expr));
                if (net.value(p) == utils::True)
                { // we update the upper bound..
                    if (!assert_upper(slack, c_right, {}))
                        throw unsolvable_exception();
                }
                else
                { // we add the assertion to the list of assertions..
                    LOG_TRACE("[" << to_string(p) << "] x" << std::to_string(slack) << " <= " << to_string(c_right));
                    v_asrts[variable(p)].emplace(new la_assertion(p, slack, op::leq, c_right));
                    bind(variable(p)); // we get notified when the slack variable changes..
                }
            }
            }
        }
        }
    }

    bool la_theory::propagate(const utils::lit &p) noexcept
    {
        if (net.value(variable(p)) == utils::True)
            for (const auto &asrt : v_asrts[variable(p)])
                switch (asrt->o)
                {
                case op::leq:
                    if (!assert_upper(asrt->x, asrt->v, {p}))
                        return false;
                    break;
                case op::geq:
                    if (!assert_lower(asrt->x, asrt->v, {p}))
                        return false;
                    break;
                }
        return true;
    }

    bool la_theory::check() noexcept
    {
        while (true)
        {
            // we search for a variable whose value is not within its bounds..
            const auto &x_i_it = std::find_if(tableau.cbegin(), tableau.cend(), [this](const auto &v)
                                              { return value(v.first) < lb(v.first) || value(v.first) > ub(v.first); });
            if (x_i_it == tableau.cend())
                return true; // all the variables are within their bounds..

            const auto x_i = x_i_it->first;    // we select the variable `x_i`..
            const auto &l = x_i_it->second->l; // we select the linear expression `x_i = ...`..
            if (value(x_i) < lb(x_i))
            { // the value of `x_i` is below its lower bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<utils::var, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && value(v.first) < ub(v.first)) || (is_negative(l.vars.at(v.first)) && value(v.first) > lb(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to increase the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, lb(x_i));
                else
                { // we generate an explanation for the conflict..
                    assert(cnfl.empty());
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            for (const auto &w : c_bounds[ub_index(v)].reason)
                                cnfl.push_back(!w);
                        else if (is_negative(c))
                            for (const auto &w : c_bounds[lb_index(v)].reason)
                                cnfl.push_back(!w);
                    for (const auto &w : c_bounds[lb_index(x_i)].reason)
                        cnfl.push_back(!w);
                    return false;
                }
            }
            else if (value(x_i) > ub(x_i))
            { // the value of `x_i` is above its upper bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<utils::var, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && value(v.first) > lb(v.first)) || (is_negative(l.vars.at(v.first)) && value(v.first) < ub(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to decrease the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, ub(x_i));
                else
                { // we generate an explanation for the conflict..
                    assert(cnfl.empty());
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            for (const auto &w : c_bounds[lb_index(v)].reason)
                                cnfl.push_back(!w);
                        else if (is_negative(c))
                            for (const auto &w : c_bounds[ub_index(v)].reason)
                                cnfl.push_back(!w);
                    for (const auto &w : c_bounds[ub_index(x_i)].reason)
                        cnfl.push_back(!w);
                    return false;
                }
            }
        }
    }

    void la_theory::push() noexcept { layers.push_back({}); }

    void la_theory::pop() noexcept
    { // we restore the bounds of the variables to the previous state..
        for (const auto &[i, b] : layers.back())
            c_bounds[i] = b;
        layers.pop_back();
    }

    bool la_theory::assert_lower(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept
    {
        LOG_TRACE("x" << std::to_string(x_i) << " >= " << to_string(val));
        assert(std::all_of(r.cbegin(), r.cend(), [this](const auto &lit)
                           { return net.value(lit) != utils::Undefined; })); // all the literals in the reason must be assigned..
        if (val <= lb(x_i))
            return true; // the assertion is already satisfied..
        else if (val > ub(x_i))
        { // the assertion introduces a conflict..
            assert(cnfl.empty());
            for (const auto &w : r) // either the assertion is false..
                cnfl.push_back(!w);
            for (const auto &w : c_bounds[ub_index(x_i)].reason) // or the reason for the upper bound is false..
                cnfl.push_back(!w);
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(lb_index(x_i), bound{lb(x_i), c_bounds[lb_index(x_i)].reason});
            c_bounds[lb_index(x_i)] = {val, r}; // we update the lower bound of the variable..

            if (vals[x_i] < val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                switch (c->o)
                {
                case leq:
                    if (auto c_b = net.value(c->b); c_b != utils::False && c_bounds[lb_index(c->x)].value >= c->v)
                    { // either the literal `b` is false or the (precomputed) reason for the lower bound of `x` is false..
                        assert(cnfl.empty());
                        cnfl.push_back(!c->b);
                        for (const auto &w : c_bounds[lb_index(c->x)].reason)
                            cnfl.push_back(!w);
                        switch (c_b)
                        {
                        case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                            return false;
                        case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> ![x <= v]..
                            record(std::move(cnfl));
                            break;
                        }
                    }
                    break;
                case geq:
                    if (auto c_b = net.value(c->b); c_b != utils::True && c_bounds[lb_index(c->x)].value > c->v)
                    { // either the literal `b` is true or the (precomputed) reason for the lower bound of `x` is false..
                        assert(cnfl.empty());
                        cnfl.push_back(c->b);
                        for (const auto &w : c_bounds[lb_index(c->x)].reason)
                            cnfl.push_back(!w);
                        switch (c_b)
                        {
                        case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                            return false;
                        case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> [x >= v]..
                            record(std::move(cnfl));
                            break;
                        }
                    }
                    break;
                }

            // bound propagation..
            for (const auto &c : t_watches[x_i])
            {
                utils::inf_rational lb_v(tableau.at(c)->l.known_term); // the lower bound of the variable `v`..
                std::vector<utils::lit> r_lb;                          // the reason for the lower bound of the variable..
                for (const auto &[v, c] : tableau.at(c)->l.vars)
                    if (is_positive(c))
                    {
                        lb_v += lb(v) * c;
                        if (is_infinite(lb_v))
                            break;
                        for (const auto &w : c_bounds[lb_index(v)].reason)
                            r_lb.push_back(w);
                    }
                    else
                    { // the coefficient is negative..
                        lb_v += ub(v) * c;
                        if (is_infinite(lb_v))
                            break;
                        for (const auto &w : c_bounds[ub_index(v)].reason)
                            r_lb.push_back(w);
                    }
                if (!is_infinite(lb_v) && !assert_lower(c, lb_v, r_lb))
                    return false;
            }
            return true;
        }
    }
    bool la_theory::assert_upper(const utils::var x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept
    {
        LOG_TRACE("x" << std::to_string(x_i) << " <= " << to_string(val));
        assert(std::all_of(r.cbegin(), r.cend(), [this](const auto &lit)
                           { return net.value(lit) != utils::Undefined; })); // all the literals in the reason must be assigned..
        if (val >= ub(x_i))
            return true; // the assertion is already satisfied..
        else if (val < lb(x_i))
        { // the assertion introduces a conflict..
            assert(cnfl.empty());
            for (const auto &w : r) // either the assertion is false..
                cnfl.push_back(!w);
            for (const auto &w : c_bounds[lb_index(x_i)].reason) // or the reason for the lower bound is false..
                cnfl.push_back(!w);
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(ub_index(x_i), bound{ub(x_i), c_bounds[ub_index(x_i)].reason});
            c_bounds[ub_index(x_i)] = {val, r}; // we update the upper bound of the variable..

            if (vals[x_i] > val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                switch (c->o)
                {
                case leq:
                    if (auto c_b = net.value(c->b); c_b != utils::True && c_bounds[ub_index(c->x)].value <= c->v)
                    { // either the literal `b` is true or the (precomputed) reason for the upper bound of `x` is false..
                        assert(cnfl.empty());
                        cnfl.push_back(c->b);
                        for (const auto &w : c_bounds[ub_index(c->x)].reason)
                            cnfl.push_back(!w);
                        switch (c_b)
                        {
                        case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                            return false;
                        case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> [x <= v]..
                            record(std::move(cnfl));
                            break;
                        }
                    }
                    break;
                case geq:
                    if (auto c_b = net.value(c->b); c_b != utils::False && c_bounds[ub_index(c->x)].value < c->v)
                    { // either the literal `b` is false or the (precomputed) reason for the upper bound of `x` is false..
                        assert(cnfl.empty());
                        cnfl.push_back(!c->b);
                        for (const auto &w : c_bounds[ub_index(c->x)].reason)
                            cnfl.push_back(!w);
                        switch (c_b)
                        {
                        case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                            return false;
                        case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> ![x >= v]..
                            record(std::move(cnfl));
                            break;
                        }
                    }
                    break;
                }

            // bound propagation..
            for (const auto &c : t_watches[x_i])
            {
                utils::inf_rational ub_v(tableau.at(c)->l.known_term); // the upper bound of the variable `v`..
                std::vector<utils::lit> r_ub;                          // the reason for the upper bound of the variable..
                for (const auto &[v, c] : tableau.at(c)->l.vars)
                    if (is_positive(c))
                    {
                        ub_v += ub(v) * c;
                        if (is_infinite(ub_v))
                            break;
                        for (const auto &w : c_bounds[ub_index(v)].reason)
                            r_ub.push_back(w);
                    }
                    else
                    { // the coefficient is negative..
                        ub_v += lb(v) * c;
                        if (is_infinite(ub_v))
                            break;
                        for (const auto &w : c_bounds[lb_index(v)].reason)
                            r_ub.push_back(w);
                    }
                if (!is_infinite(ub_v) && !assert_upper(c, ub_v, r_ub))
                    return false;
            }
            return true;
        }
    }

    void la_theory::update(const utils::var x_i, const utils::inf_rational &v) noexcept
    {
        assert(!is_basic(x_i)); // the variable must not be basic..

        // the tableau rows containing `x_i` as a non-basic variable..
        for (const auto &c : t_watches[x_i])
        { // x_j = x_j + a_ji(v - x_i)..
            vals[c] += tableau.at(c)->l.vars.at(x_i) * (v - vals[x_i]);
            FIRE_ON_CHANGE(c);
        }
        // x_i = v..
        vals[x_i] = v;
        FIRE_ON_CHANGE(x_i);
    }
    void la_theory::pivot_and_update(const utils::var x_i, const utils::var x_j, const utils::inf_rational &v) noexcept
    {
        assert(is_basic(x_i));                      // the variable must be basic..
        assert(!is_basic(x_j));                     // the variable must not be basic..
        assert(tableau.at(x_i)->l.vars.count(x_j)); // the variable `x_j` must be in the row of `x_i`..

        const utils::inf_rational theta = (v - vals[x_i]) / tableau.at(x_i)->l.vars.at(x_j);
        assert(!is_infinite(theta));

        // x_i = v
        vals[x_i] = v;
        FIRE_ON_CHANGE(x_i);

        // x_j += theta
        vals[x_j] += theta;
        FIRE_ON_CHANGE(x_j);

        // the tableau rows containing `x_j` as a non-basic variable..
        for (const auto &c : t_watches[x_j])
            if (c != x_i)
            { // x_k += a_kj * theta..
                vals[c] += tableau.at(c)->l.vars.at(x_j) * theta;
                FIRE_ON_CHANGE(c);
            }

        pivot(x_i, x_j);
    }
    void la_theory::pivot(const utils::var x_i, const utils::var x_j) noexcept
    {
        assert(is_basic(x_i));                      // the variable must be basic..
        assert(!is_basic(x_j));                     // the variable must not be basic..
        assert(tableau.at(x_i)->l.vars.count(x_j)); // the variable `x_j` must be in the row of `x_i`..
        assert(t_watches[x_i].empty());             // the variable `x_i` must not be in any other row of the tableau..

        // we remove the row from the watches
        for ([[maybe_unused]] const auto &[v, c] : tableau[x_i]->l.vars)
        {
            assert(t_watches[v].count(x_i));
            t_watches[v].erase(x_i);
        }

        // we rewrite `x_i = ...` as `x_j = ...`
        utils::lin l = std::move(tableau[x_i]->l);
        utils::rational cc = l.vars.at(x_j);
        l.vars.erase(x_j);
        l /= -cc;
        l.vars.emplace(x_i, utils::rational::one / cc);
        tableau.erase(x_i);

        // we update the rows that contain `x_j`
        for (auto &r : t_watches[x_j])
        {
            auto &c_l = tableau[r]->l;
            assert(c_l.known_term == utils::rational::zero);
            cc = c_l.vars.at(x_j);
            c_l.vars.erase(x_j);
            for (const auto &[v, c] : l.vars)
                if (const auto trm_it = c_l.vars.find(v); trm_it == c_l.vars.cend())
                {                                // `v` is not in the linear expression of `r`, so we add it
                    c_l.vars.emplace(v, c * cc); // we add `c * cc` to the linear expression of `r`
                    t_watches[v].insert(r);      // we add `r` to the watches of `v`
                }
                else
                {
                    trm_it->second += c * cc;
                    if (trm_it->second == 0)
                    {                           // if the coefficient of `v` is zero, we remove the term from the linear expression
                        c_l.vars.erase(trm_it); // we remove `v` from the linear expression of `r`
                        t_watches[v].erase(r);  // we remove `r` from the watches of `v`
                    }
                }
            LOG_TRACE("x" << std::to_string(r) << " = " << to_string(c_l));
        }
        t_watches[x_j].clear();

        // we add the new row `x_j = ...`
        new_row(x_j, std::move(l));
    }
    void la_theory::new_row(const utils::var x_i, utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        LOG_TRACE("x" << std::to_string(x_i) << " = " << to_string(xpr));
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, new la_eq(x_i, std::move(xpr)));
    }

    [[nodiscard]] std::ostream &operator<<(std::ostream &os, const la_theory &th)
    {
        os << "Variables:\n";
        for (size_t i = 0; i < th.vals.size(); ++i)
        {
            os << "x" << std::to_string(i) << " = " << to_string(th.vals[i]);
            if (th.is_int(i))
                os << " (int)";
            os << " [" << to_string(th.lb(i)) << ", " << to_string(th.ub(i)) << "]\n";
        }
        os << "Assertions:\n";
        for (const auto &[v, asrts] : th.v_asrts)
            for (const auto &asrt : asrts)
            {
                os << "[" << to_string(asrt->b) << "] x" << std::to_string(v) << " ";
                switch (asrt->o)
                {
                case op::leq:
                    os << "<= ";
                    break;
                case op::geq:
                    os << ">= ";
                    break;
                }
                os << to_string(asrt->v) << "\n";
            }
        os << "Tableau:\n";
        for (const auto &[v, c] : th.tableau)
            os << "x" << std::to_string(v) << " = " << to_string(c->l) << "\n";
        return os;
    }
} // namespace semitone