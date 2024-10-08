#include <algorithm>
#include <cassert>
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "lra_assertion.hpp"
#include "lra_eq.hpp"

#ifdef BUILD_LISTENERS
#include "lra_value_listener.hpp"
#define FIRE_ON_VALUE_CHANGED(var)                                       \
    if (const auto &at_v = listening.find(var); at_v != listening.end()) \
        for (auto &l : at_v->second)                                     \
            l->on_lra_value_changed(var);
#else
#define FIRE_ON_VALUE_CHANGED(var)
#endif

namespace semitone
{
    VARIABLE_TYPE lra_theory::new_var() noexcept
    {
        auto var = vals.size();
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::negative_infinite), utils::TRUE_lit});
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::positive_infinite), utils::TRUE_lit});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        exprs.emplace("x" + std::to_string(var), var);
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }
    VARIABLE_TYPE lra_theory::new_var(const utils::lin &&l) noexcept
    {
        assert(get_sat().root_level());
        const auto s_expr = to_string(l);
        if (const auto it = exprs.find(s_expr); it != exprs.cend())
            return it->second;

        const auto slack = new_var();
        c_bounds[lb_index(slack)] = {lb(l), utils::TRUE_lit}; // we set the lower bound of the slack variable to the lower bound of the linear expression
        c_bounds[ub_index(slack)] = {ub(l), utils::TRUE_lit}; // we set the upper bound of the slack variable to the upper bound of the linear expression
        vals[slack] = value(l);                               // we set the value of the slack variable to the value of the linear expression
        exprs.emplace(s_expr, slack);                         // we add the linear expression to the expressions
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

        // we create a slack variable from the current expression (notice that the variable can be reused)..
        const auto slack = new_var(std::move(expr));
        if (ub(slack) <= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (lb(slack) > c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(slack) + " <= " + to_string(c_right);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
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

        // we create a slack variable from the current expression (notice that the variable can be reused)..
        const auto slack = new_var(std::move(expr));
        if (ub(slack) <= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (lb(slack) > c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(slack) + " <= " + to_string(c_right);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
        v_asrts.emplace(ctr, std::make_unique<lra_leq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }
    utils::lit lra_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept { return get_sat().new_conj({new_leq(left, right), new_geq(left, right)}); }
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

        // we create a slack variable from the current expression (notice that the variable can be reused)..
        const auto slack = new_var(std::move(expr));
        if (lb(slack) >= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (ub(slack) < c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(slack) + " >= " + to_string(c_right);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
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

        // we create a slack variable from the current expression (notice that the variable can be reused)..
        const auto slack = new_var(std::move(expr));
        if (lb(slack) >= c_right)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (ub(slack) < c_right)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(slack) + " >= " + to_string(c_right);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
        v_asrts.emplace(ctr, std::make_unique<lra_geq>(*this, ctr_lit, new_var(std::move(expr)), c_right));
        return ctr_lit;
    }

    bool lra_theory::assert_lower(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept
    {
        assert(get_sat().value(p) != utils::Undefined); // the literal must be assigned..
        assert(cnfl.empty());

        if (val <= lb(x_i)) // the assertion is already satisfied..
            return true;
        else if (val > ub(x_i)) // the assertion introduces a conflict..
        {
            cnfl.push_back(!p);                              // either the assertion is unsatisfable..
            cnfl.push_back(!c_bounds[ub_index(x_i)].reason); // or the reason for the upper bound is false..
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(lb_index(x_i), bound{lb(x_i), c_bounds[lb_index(x_i)].reason});
            c_bounds[lb_index(x_i)] = {val, p}; // we update the lower bound of the variable..

            if (vals[x_i] < val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                if (!c.get().propagate_lb(val))
                    return false;
            // bound propagation..
            for (const auto &c : t_watches[x_i])
                if (!tableau.at(c)->propagate_lb(x_i))
                    return false;

            return true;
        }
    }

    bool lra_theory::assert_upper(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept
    {
        assert(get_sat().value(p) != utils::Undefined); // the literal must be assigned..
        assert(cnfl.empty());
        if (val >= ub(x_i)) // the assertion is already satisfied..
            return true;
        else if (val < lb(x_i)) // the assertion introduces a conflict..
        {
            cnfl.push_back(!p);                              // either the assertion is unsatisfable..
            cnfl.push_back(!c_bounds[lb_index(x_i)].reason); // or the reason for the lower bound is false..
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(ub_index(x_i), bound{ub(x_i), c_bounds[ub_index(x_i)].reason});
            c_bounds[ub_index(x_i)] = {val, p}; // we update the upper bound of the variable..

            if (vals[x_i] > val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                if (!c.get().propagate_ub(val))
                    return false;
            // bound propagation..
            for (const auto &c : t_watches[x_i])
                if (!tableau.at(c)->propagate_ub(x_i))
                    return false;

            return true;
        }
    }

    bool lra_theory::set_lb(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept { return assert_lower(x_i, val, p) && get_sat().propagate(); }
    bool lra_theory::set_ub(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept { return assert_upper(x_i, val, p) && get_sat().propagate(); }
    bool lra_theory::set_eq(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const utils::lit &p) noexcept { return assert_lower(x_i, val, p) && assert_upper(x_i, val, p) && get_sat().propagate(); }

    void lra_theory::update(const VARIABLE_TYPE x_i, const utils::inf_rational &val) noexcept
    {
        assert(!is_basic(x_i)); // the variable must not be basic..

        // the tableau rows containing `x_i` as a non-basic variable..
        for (const auto &c : t_watches[x_i])
        { // x_j = x_j + a_ji(v - x_i)..
            vals[c] += tableau.at(c)->get_lin().vars.at(x_i) * (val - vals[x_i]);
            FIRE_ON_VALUE_CHANGED(c);
        }
        // x_i = v..
        vals[x_i] = val;
        FIRE_ON_VALUE_CHANGED(x_i);
    }

    void lra_theory::pivot_and_update(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j, const utils::inf_rational &v) noexcept
    {
        assert(is_basic(x_i));                              // the variable must be basic..
        assert(!is_basic(x_j));                             // the variable must not be basic..
        assert(tableau.at(x_i)->get_lin().vars.count(x_j)); // the variable `x_j` must be in the row of `x_i`..

        const utils::inf_rational theta = (v - vals[x_i]) / tableau.at(x_i)->get_lin().vars.at(x_j);
        assert(!is_infinite(theta));

        // x_i = v
        vals[x_i] = v;
        FIRE_ON_VALUE_CHANGED(x_i);

        // x_j += theta
        vals[x_j] += theta;
        FIRE_ON_VALUE_CHANGED(x_j);

        // the tableau rows containing `x_j` as a non-basic variable..
        for (const auto &c : t_watches[x_j])
            if (c != x_i)
            { // x_k += a_kj * theta..
                vals[c] += tableau.at(c)->get_lin().vars.at(x_j) * theta;
                FIRE_ON_VALUE_CHANGED(c);
            }

        pivot(x_i, x_j);
    }

    void lra_theory::pivot(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j) noexcept
    {
        assert(is_basic(x_i));                              // the variable must be basic..
        assert(!is_basic(x_j));                             // the variable must not be basic..
        assert(tableau.at(x_i)->get_lin().vars.count(x_j)); // the variable `x_j` must be in the row of `x_i`..
        assert(t_watches[x_i].empty());                     // the variable `x_i` must not be in any other row of the tableau..

        // we remove the row from the watches
        for ([[maybe_unused]] const auto &[v, c] : tableau[x_i]->get_lin().vars)
        {
            assert(t_watches[v].count(x_i));
            t_watches[v].erase(x_i);
        }

        // we rewrite `x_i = ...` as `x_j = ...`
        utils::lin l = std::move(tableau[x_i]->get_lin());
        utils::rational cc = l.vars.at(x_j);
        l.vars.erase(x_j);
        l /= -cc;
        l.vars.emplace(x_i, utils::rational::one / cc);
        tableau.erase(x_i);

        // we update the rows that contain `x_j`
        for (auto &r : t_watches[x_j])
        {
            auto &c_l = tableau[r]->get_lin();
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
        }
        t_watches[x_j].clear();

        // we add the new row `x_j = ...`
        new_row(x_j, std::move(l));
    }

    void lra_theory::new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, std::make_unique<lra_eq>(*this, x_i, std::move(xpr)));
    }

    bool lra_theory::propagate(const utils::lit &p) noexcept
    {
        assert(cnfl.empty());
        const auto &a = v_asrts[variable(p)];
        switch (get_sat().value(a->get_lit()))
        {
        case utils::True: // direct assertion..
            if (!((a->get_op() == leq) ? assert_upper(a->get_var(), a->get_val(), p) : assert_lower(a->get_var(), a->get_val(), p)))
                return false;
            break;
        case utils::False: // negated assertion..
            if (!((a->get_op() == leq) ? assert_lower(a->get_var(), a->get_val() + utils::inf_rational::epsilon, p) : assert_upper(a->get_var(), a->get_val() - utils::inf_rational::epsilon, p)))
                return false;
            break;
        }
        return true;
    }
    bool lra_theory::check() noexcept
    {
        assert(cnfl.empty());
        while (true)
        {
            // we search for a variable whose value is not within its bounds..
            const auto &x_i_it = std::find_if(tableau.cbegin(), tableau.cend(), [this](const auto &v)
                                              { return value(v.first) < lb(v.first) || value(v.first) > ub(v.first); });
            if (x_i_it == tableau.cend())
                return true; // all the variables are within their bounds..

            const auto x_i = x_i_it->first;            // we select the variable `x_i`..
            const auto &l = x_i_it->second->get_lin(); // we select the linear expression `x_i = ...`..
            if (value(x_i) < lb(x_i))
            { // the value of `x_i` is below its lower bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<VARIABLE_TYPE, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && value(v.first) < ub(v.first)) || (is_negative(l.vars.at(v.first)) && value(v.first) > lb(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to increase the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, lb(x_i));
                else
                { // we generate an explanation for the conflict..
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            cnfl.push_back(!c_bounds[ub_index(v)].reason);
                        else if (is_negative(c))
                            cnfl.push_back(!c_bounds[lb_index(v)].reason);
                    cnfl.push_back(!c_bounds[lb_index(x_i)].reason);
                    return false;
                }
            }
            else if (value(x_i) > ub(x_i))
            { // the value of `x_i` is above its upper bound..
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<VARIABLE_TYPE, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && value(v.first) > lb(v.first)) || (is_negative(l.vars.at(v.first)) && value(v.first) < ub(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to decrease the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, ub(x_i));
                else
                { // we generate an explanation for the conflict..
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            cnfl.push_back(!c_bounds[lb_index(v)].reason);
                        else if (is_negative(c))
                            cnfl.push_back(!c_bounds[ub_index(v)].reason);
                    cnfl.push_back(!c_bounds[ub_index(x_i)].reason);
                    return false;
                }
            }
        }
    }
    void lra_theory::push() noexcept { layers.emplace_back(); }
    void lra_theory::pop() noexcept
    { // we restore the bounds of the variables to the previous state..
        for (const auto &[i, b] : layers.back())
            c_bounds[i] = b;
        layers.pop_back();
    }

#ifdef BUILD_LISTENERS
    void lra_theory::add_listener(lra_value_listener &l) noexcept
    {
        l.th = this;
        listeners.insert(&l);
    }
    void lra_theory::remove_listener(lra_value_listener &l) noexcept
    {
        l.th = nullptr;
        for (auto v : l.listening)
        {
            listening[v].erase(&l);
            if (listening[v].empty())
                listening.erase(v);
        }
        listeners.erase(&l);
    }
#endif
} // namespace semitone