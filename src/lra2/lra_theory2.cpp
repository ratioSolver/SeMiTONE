#include "lra_theory2.hpp"
#include "sat_core.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cassert>

#ifdef BUILD_LISTENERS
#include "lra_value_listener2.hpp"
#define FIRE_ON_VALUE_CHANGED(var)                                       \
    if (const auto &at_v = listening.find(var); at_v != listening.end()) \
        for (auto &l : at_v->second)                                     \
            l->on_lra_value_changed(var);
#else
#define FIRE_ON_VALUE_CHANGED(var)
#endif

namespace semitone
{
    lra_theory2::~lra_theory2()
    {
        LOG_TRACE("Destroying the LRA theory");
#ifdef BUILD_LISTENERS
        for (auto l : listeners)
            l->th = nullptr;
#endif
    }

    VARIABLE_TYPE lra_theory2::new_var(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        exprs.emplace("x" + std::to_string(var), var);
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }
    VARIABLE_TYPE lra_theory2::new_var(const utils::lin &&l) noexcept
    {
        assert(get_sat().root_level());
        const auto s_expr = to_string(l);
        if (const auto it = exprs.find(s_expr); it != exprs.cend())
            return it->second;

        const auto slack = new_var();

        utils::inf_rational val(l.known_term), lb(l.known_term), ub(l.known_term);
        std::vector<utils::lit> lb_reason, ub_reason;
        for (const auto &[v, c] : l.vars)
        {
            val += c * vals[v];
            lb += (is_positive(c) ? c_bounds[lb_index(v)].value : c_bounds[ub_index(v)].value) * c;
            lb_reason.insert(lb_reason.end(), c_bounds[lb_index(v)].reason.cbegin(), c_bounds[lb_index(v)].reason.cend());
            ub += (is_positive(c) ? c_bounds[ub_index(v)].value : c_bounds[lb_index(v)].value) * c;
            ub_reason.insert(ub_reason.end(), c_bounds[ub_index(v)].reason.cbegin(), c_bounds[ub_index(v)].reason.cend());
        }

        c_bounds[lb_index(slack)] = {lb, lb_reason}; // we set the lower bound of the slack variable to the lower bound of the linear expression
        c_bounds[ub_index(slack)] = {ub, ub_reason}; // we set the upper bound of the slack variable to the upper bound of the linear expression
        vals[slack] = val;                           // we set the value of the slack variable to the value of the linear expression
        exprs.emplace(s_expr, slack);                // we add the linear expression to the expressions
        new_row(slack, std::move(l));                // we add the new row `slack = ...` to the tableau
        return slack;
    }

    [[nodiscard]] utils::lit lra_theory2::new_leq(const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept
    {
        assert(get_sat().root_level());
        if (ub(x) <= v)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (lb(x) > v)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(x) + " <= " + to_string(v);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
        v_asrts.emplace(ctr, std::make_unique<lra_assertion>(ctr_lit, x, op::leq, v));
        return ctr_lit;
    }
    [[nodiscard]] utils::lit lra_theory2::new_geq(const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept
    {
        assert(get_sat().root_level());
        if (lb(x) >= v)
            return utils::TRUE_lit; // the constraint is already satisfied..
        else if (ub(x) < v)
            return utils::FALSE_lit; // the constraint is unsatisfable..

        const auto s_asrt = "x" + std::to_string(x) + " >= " + to_string(v);
        if (const auto asrt_it = s_asrts.find(s_asrt); asrt_it != s_asrts.cend())
            return asrt_it->second;

        // we create a new control variable..
        const auto ctr = get_sat().new_var();
        const utils::lit ctr_lit(ctr);
        bind(ctr);
        s_asrts.emplace(s_asrt, ctr_lit);
        v_asrts.emplace(ctr, std::make_unique<lra_assertion>(ctr_lit, x, op::geq, v));
        return ctr_lit;
    }

    [[nodiscard]] utils::lit lra_theory2::new_lt(const utils::lin &left, const utils::lin &right) noexcept
    {
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
                expr += c * tableau.at(v)->l;
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant
            return expr.known_term < utils::rational::zero ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        { // the expression is an inequality with a single variable
            const auto [v, c] = *expr.vars.cbegin();
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, -1) / c;
            if (c > 0)
                return new_leq(v, c_right);
            else
                return new_geq(v, c_right);
        }
        default:
        { // the expression is an inequality with multiple variables
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term, -1);
            expr.known_term = utils::rational::zero;
            return new_leq(new_var(std::move(expr)), c_right);
        }
        }
    }
    [[nodiscard]] utils::lit lra_theory2::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
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
                expr += c * tableau.at(v)->l;
            }

        switch (expr.vars.size())
        {
        case 0: // the expression is a constant
            return expr.known_term <= utils::rational::zero ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        { // the expression is an inequality with a single variable
            const auto [v, c] = *expr.vars.cbegin();
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term) / c;
            if (c > 0)
                return new_leq(v, c_right);
            else
                return new_geq(v, c_right);
        }
        default:
        { // the expression is an inequality with multiple variables
            const utils::inf_rational c_right = utils::inf_rational(-expr.known_term);
            expr.known_term = utils::rational::zero;
            return new_leq(new_var(std::move(expr)), c_right);
        }
        }
    }

#ifdef BUILD_LISTENERS
    void lra_theory2::add_listener(lra_value_listener2 &l) noexcept
    {
        l.th = this;
        listeners.insert(&l);
    }
    void lra_theory2::remove_listener(lra_value_listener2 &l) noexcept
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

    [[nodiscard]] std::pair<utils::inf_rational, std::vector<utils::lit>> lra_theory2::lb_and_reason(const utils::lin &l) const noexcept
    {
        utils::inf_rational b(l.known_term);
        std::vector<utils::lit> cnfl;
        for (const auto &[v, c] : l.vars)
            if (is_positive(c))
            {
                b += lb(v) * c;
                if (is_infinite(b))
                    return {b, {}};
                for (const auto &w : c_bounds[lb_index(v)].reason)
                    cnfl.push_back(w);
            }
            else
            {
                b += ub(v) * c;
                if (is_infinite(b))
                    return {b, {}};
                for (const auto &w : c_bounds[ub_index(v)].reason)
                    cnfl.push_back(w);
            }
        return {b, cnfl};
    }
    [[nodiscard]] std::pair<utils::inf_rational, std::vector<utils::lit>> lra_theory2::ub_and_reason(const utils::lin &l) const noexcept
    {
        utils::inf_rational b(l.known_term);
        std::vector<utils::lit> cnfl;
        for (const auto &[v, c] : l.vars)
            if (is_positive(c))
            {
                b += ub(v) * c;
                if (is_infinite(b))
                    return {b, {}};
                for (const auto &w : c_bounds[ub_index(v)].reason)
                    cnfl.push_back(w);
            }
            else
            {
                b += lb(v) * c;
                if (is_infinite(b))
                    return {b, {}};
                for (const auto &w : c_bounds[lb_index(v)].reason)
                    cnfl.push_back(w);
            }
        return {b, cnfl};
    }

    [[nodiscard]] bool lra_theory2::assert_lower(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept
    {
        assert(std::all_of(r.cbegin(), r.cend(), [this](const auto &lit)
                           { return get_sat().value(lit) != utils::Undefined; })); // the literals must be assigned..
        if (val <= lb(x_i))                                                        // the assertion is already satisfied..
            return true;
        else if (val > ub(x_i))
        { // the assertion introduces a conflict..
            std::vector<utils::lit> cnfl;
            for (const auto &w : r) // either the assertion is false..
                cnfl.push_back(!w);
            for (const auto &w : c_bounds[ub_index(x_i)].reason) // or the reason for the upper bound is false..
                cnfl.push_back(!w);
            set_theory_conflict(std::move(cnfl));
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(lb_index(x_i), bound{lb(x_i), c_bounds[lb_index(x_i)].reason});
            c_bounds[lb_index(x_i)] = {val, r}; // we update the lower bound of the variable..

            if (vals[x_i] < val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            prop_queue.push(var_update{x_i, geq});
            return true;
        }
    }
    [[nodiscard]] bool lra_theory2::assert_upper(const VARIABLE_TYPE x_i, const utils::inf_rational &val, const std::vector<utils::lit> &r) noexcept
    {
        assert(std::all_of(r.cbegin(), r.cend(), [this](const auto &lit)
                           { return get_sat().value(lit) != utils::Undefined; })); // the literals must be assigned..
        if (val >= ub(x_i))                                                        // the assertion is already satisfied..
            return true;
        else if (val < lb(x_i))
        { // the assertion introduces a conflict..
            std::vector<utils::lit> cnfl;
            for (const auto &w : r) // either the assertion is false..
                cnfl.push_back(!w);
            for (const auto &w : c_bounds[lb_index(x_i)].reason) // or the reason for the lower bound is false..
                cnfl.push_back(!w);
            set_theory_conflict(std::move(cnfl));
            return false;
        }
        else
        {
            if (!layers.empty()) // we store the current bounds for backtracking..
                layers.back().emplace(ub_index(x_i), bound{ub(x_i), c_bounds[ub_index(x_i)].reason});
            c_bounds[ub_index(x_i)] = {val, r}; // we update the upper bound of the variable..

            if (vals[x_i] > val && !is_basic(x_i))
                update(x_i, val); // we set the value of `x_i` to `val` and update all the basic variables which are related to `x_i` by the tableau..

            prop_queue.push(var_update{x_i, leq});
            return true;
        }
    }

    bool lra_theory2::propagate() noexcept
    {
        while (!prop_queue.empty())
        {
            const auto [x, o] = prop_queue.front();
            prop_queue.pop();
            switch (o)
            {
            case leq: // an upper bound has been updated..
                      // unate propagation..
                for (const auto &c : a_watches[x])
                    switch (c.get().o)
                    {
                    case leq:
                        if (auto c_b = get_sat().value(c.get().b); c_b != utils::False && c_bounds[ub_index(c.get().x)].value <= c.get().v)
                        { // either the literal `b` is false or the (precomputed) reason for the lower bound of `x` is false..
                            std::vector<utils::lit> cnfl;
                            cnfl.push_back(!c.get().b);
                            for (const auto &w : c_bounds[ub_index(c.get().x)].reason)
                                cnfl.push_back(!w);
                            switch (c_b)
                            {
                            case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                set_theory_conflict(std::move(cnfl));
                                return false;
                            case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> [x <= v]..
                                record(std::move(cnfl));
                                break;
                            }
                        }
                        break;
                    case geq:
                        if (auto c_b = get_sat().value(c.get().b); c_b != utils::True && c_bounds[ub_index(c.get().x)].value < c.get().v)
                        { // either the literal `b` is true or the (precomputed) reason for the lower bound of `x` is false..
                            std::vector<utils::lit> cnfl;
                            cnfl.push_back(c.get().b);
                            for (const auto &w : c_bounds[ub_index(c.get().x)].reason)
                                cnfl.push_back(!w);
                            switch (c_b)
                            {
                            case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                set_theory_conflict(std::move(cnfl));
                                return false;
                            case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> ![x >= v]..
                                record(std::move(cnfl));
                                break;
                            }
                        }
                        break;
                    }
                // bound propagation..
                if (tableau.find(x) != tableau.cend())
                { // bound propagation for the basic variable `x`..
                  // we look for tighter bounds..
                    utils::lin l = utils::lin(x, utils::rational::one) - tableau.at(x)->l;
                    for (const auto &[v, c] : l.vars)
                    {
                        utils::lin c_l = l / c;
                        c_l.vars.erase(v);
                        if (is_positive(c))
                        {
                            const auto [lb_v, r_lb] = lb_and_reason(c_l);
                            if (!assert_lower(v, lb_v, r_lb))
                                return false;
                        }
                        else
                        {
                            const auto [ub_v, r_ub] = ub_and_reason(c_l);
                            if (!assert_upper(v, ub_v, r_ub))
                                return false;
                        }
                    }
                }
                else // bound propagation for the non-basic variable `x`..
                    for (const auto &c : t_watches[x])
                    { // we look for tighter bounds..
                        utils::lin l = utils::lin(c, utils::rational::one) - tableau.at(c)->l;
                        for (const auto &[v, c] : tableau.at(c)->l.vars)
                            if (v != x)
                            {
                                utils::lin c_l = l / c;
                                c_l.vars.erase(v);
                                if (is_positive(c))
                                {
                                    const auto [ub_v, r_ub] = ub_and_reason(c_l);
                                    if (!assert_upper(v, ub_v, r_ub))
                                        return false;
                                }
                                else
                                {
                                    const auto [lb_v, r_lb] = lb_and_reason(c_l);
                                    if (!assert_lower(v, lb_v, r_lb))
                                        return false;
                                }
                            }
                    }
                break;
            case geq: // a lower bound has been updated..
                      // unate propagation..
                for (const auto &c : a_watches[x])
                    switch (c.get().o)
                    {
                    case leq:
                        if (auto c_b = get_sat().value(c.get().b); c_b != utils::False && c_bounds[lb_index(c.get().x)].value >= c.get().v)
                        { // either the literal `b` is false or the (precomputed) reason for the lower bound of `x` is false..
                            std::vector<utils::lit> cnfl;
                            cnfl.push_back(!c.get().b);
                            for (const auto &w : c_bounds[lb_index(c.get().x)].reason)
                                cnfl.push_back(!w);
                            switch (c_b)
                            {
                            case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                set_theory_conflict(std::move(cnfl));
                                return false;
                            case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> ![x <= v]..
                                record(std::move(cnfl));
                                break;
                            }
                        }
                        break;
                    case geq:
                        if (auto c_b = get_sat().value(c.get().b); c_b != utils::True && c_bounds[lb_index(c.get().x)].value > c.get().v)
                        { // either the literal `b` is true or the (precomputed) reason for the lower bound of `x` is false..
                            std::vector<utils::lit> cnfl;
                            cnfl.push_back(c.get().b);
                            for (const auto &w : c_bounds[lb_index(c.get().x)].reason)
                                cnfl.push_back(!w);
                            switch (c_b)
                            {
                            case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                set_theory_conflict(std::move(cnfl));
                                return false;
                            case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> [x >= v]..
                                record(std::move(cnfl));
                                break;
                            }
                        }
                        break;
                    }
                // bound propagation..
                if (tableau.find(x) != tableau.cend())
                { // bound propagation for the basic variable `x`..
                  // we look for tighter bounds..
                    utils::lin l = utils::lin(x, utils::rational::one) - tableau.at(x)->l;
                    for (const auto &[v, c] : l.vars)
                    {
                        utils::lin c_l = l / c;
                        c_l.vars.erase(v);
                        if (is_positive(c))
                        {
                            const auto [ub_v, r_ub] = ub_and_reason(c_l);
                            if (!assert_upper(v, ub_v, r_ub))
                                return false;
                        }
                        else
                        {
                            const auto [lb_v, r_lb] = lb_and_reason(c_l);
                            if (!assert_lower(v, lb_v, r_lb))
                                return false;
                        }
                    }
                }
                else // bound propagation for the non-basic variable `x`..
                    for (const auto &c : t_watches[x])
                    { // we look for tighter bounds..
                        utils::lin l = utils::lin(c, utils::rational::one) - tableau.at(c)->l;
                        for (const auto &[v, c] : tableau.at(c)->l.vars)
                            if (v != x)
                            {
                                utils::lin c_l = l / c;
                                c_l.vars.erase(v);
                                if (is_positive(c))
                                {
                                    const auto [lb_v, r_lb] = lb_and_reason(c_l);
                                    if (!assert_lower(v, lb_v, r_lb))
                                        return false;
                                }
                                else
                                {
                                    const auto [ub_v, r_ub] = ub_and_reason(c_l);
                                    if (!assert_upper(v, ub_v, r_ub))
                                        return false;
                                }
                            }
                    }
                break;
            }
        }
        return true;
    }

    void lra_theory2::update(const VARIABLE_TYPE x_i, const utils::inf_rational &v) noexcept
    {
        assert(!is_basic(x_i)); // the variable must not be basic..

        // the tableau rows containing `x_i` as a non-basic variable..
        for (const auto &c : t_watches[x_i])
        { // x_j = x_j + a_ji(v - x_i)..
            vals[c] += tableau.at(c)->l.vars.at(x_i) * (v - vals[x_i]);
            FIRE_ON_VALUE_CHANGED(c);
        }
        // x_i = v..
        vals[x_i] = v;
        FIRE_ON_VALUE_CHANGED(x_i);
    }
    void lra_theory2::pivot_and_update(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j, const utils::inf_rational &v) noexcept
    {
        assert(is_basic(x_i));                      // the variable must be basic..
        assert(!is_basic(x_j));                     // the variable must not be basic..
        assert(tableau.at(x_i)->l.vars.count(x_j)); // the variable `x_j` must be in the row of `x_i`..

        const utils::inf_rational theta = (v - vals[x_i]) / tableau.at(x_i)->l.vars.at(x_j);
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
                vals[c] += tableau.at(c)->l.vars.at(x_j) * theta;
                FIRE_ON_VALUE_CHANGED(c);
            }

        pivot(x_i, x_j);
    }
    void lra_theory2::pivot(const VARIABLE_TYPE x_i, const VARIABLE_TYPE x_j) noexcept
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
        }
        t_watches[x_j].clear();

        // we add the new row `x_j = ...`
        new_row(x_j, std::move(l));
    }
    void lra_theory2::new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, std::make_unique<lra_eq>(x_i, std::move(xpr)));
    }

    [[nodiscard]] bool lra_theory2::propagate(const utils::lit &p) noexcept
    {
        const auto &a = v_asrts[variable(p)];
        switch (get_sat().value(a->b))
        {
        case utils::True: // direct assertion..
            if (!((a->o == leq) ? assert_upper(a->x, a->v, {p}) : assert_lower(a->x, a->v, {p})))
                return false;
            break;
        case utils::False: // negated assertion..
            if (!((a->o == leq) ? assert_lower(a->x, a->v + utils::inf_rational::epsilon, {p}) : assert_upper(a->x, a->v - utils::inf_rational::epsilon, {p})))
                return false;
            break;
        }
        return true;
    }
    [[nodiscard]] bool lra_theory2::check() noexcept
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
                const auto &x_j_it = std::find_if(l.vars.cbegin(), l.vars.cend(), [l, this](const std::pair<VARIABLE_TYPE, utils::rational> &v)
                                                  { return (is_positive(l.vars.at(v.first)) && value(v.first) < ub(v.first)) || (is_negative(l.vars.at(v.first)) && value(v.first) > lb(v.first)); });
                if (x_j_it != l.vars.cend()) // var x_j can be used to increase the value of x_i..
                    pivot_and_update(x_i, x_j_it->first, lb(x_i));
                else
                { // we generate an explanation for the conflict..
                    std::vector<utils::lit> cnfl;
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            for (const auto &w : c_bounds[lb_index(v)].reason)
                                cnfl.push_back(!w);
                        else if (is_negative(c))
                            for (const auto &w : c_bounds[ub_index(v)].reason)
                                cnfl.push_back(!w);
                    for (const auto &w : c_bounds[lb_index(x_i)].reason)
                        cnfl.push_back(!w);
                    set_theory_conflict(std::move(cnfl));
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
                    std::vector<utils::lit> cnfl;
                    for (const auto &[v, c] : l.vars)
                        if (is_positive(c))
                            for (const auto &w : c_bounds[lb_index(v)].reason)
                                cnfl.push_back(!w);
                        else if (is_negative(c))
                            for (const auto &w : c_bounds[ub_index(v)].reason)
                                cnfl.push_back(!w);
                    for (const auto &w : c_bounds[ub_index(x_i)].reason)
                        cnfl.push_back(!w);
                    set_theory_conflict(std::move(cnfl));
                    return false;
                }
            }
        }
    }
    void lra_theory2::push() noexcept { layers.emplace_back(); }
    void lra_theory2::pop() noexcept
    { // we restore the bounds of the variables to the previous state..
        for (const auto &[i, b] : layers.back())
            c_bounds[i] = b;
        layers.pop_back();
    }
} // namespace semitone
