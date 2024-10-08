#include "sat_core.hpp"
#include "rdl_theory.hpp"
#include "logging.hpp"
#include <cassert>
#include <stdexcept>

#ifdef BUILD_LISTENERS
#include "rdl_value_listener.hpp"
#define FIRE_ON_VALUE_CHANGED(var)                                       \
    if (const auto &at_v = listening.find(var); at_v != listening.end()) \
        for (auto &l : at_v->second)                                     \
            l->on_rdl_value_changed(var);
#else
#define FIRE_ON_VALUE_CHANGED(var)
#endif

namespace semitone
{
    rdl_theory::rdl_theory(const size_t &size) noexcept : dists(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite))), preds(size, std::vector<VARIABLE_TYPE>(size))
    {
        assert(size > 1);
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
            std::fill(preds[i].begin(), preds[i].end(), std::numeric_limits<VARIABLE_TYPE>::max());
            preds[i][i] = i;
        }
    }
    rdl_theory::rdl_theory(const rdl_theory &orig) noexcept : n_vars(orig.n_vars), dists(orig.dists), preds(orig.preds)
    {
        for (const auto &[var, constr] : orig.var_dists)
            var_dists.emplace(var, std::make_unique<distance_constraint<utils::inf_rational>>(*constr));
        for (const auto &[from_to, constr] : orig.dist_constr)
            dist_constr.emplace(from_to, *var_dists.at(variable(constr.get().get_lit())));
        for (const auto &[from_to, constrs] : orig.dist_constrs)
            for (const auto &constr : constrs)
                dist_constrs[from_to].emplace_back(*var_dists.at(variable(constr.get().get_lit())));
    }
    rdl_theory::~rdl_theory()
    {
        LOG_DEBUG("Destroying rdl_theory");
#ifdef BUILD_LISTENERS
        for (auto l : listeners)
            l->th = nullptr;
#endif
    }

    VARIABLE_TYPE rdl_theory::new_var() noexcept
    {
        auto var = n_vars++;
        if (var >= dists.size())
            resize((dists.size() * 3) / 2 + 1);
        return var;
    }

    utils::lit rdl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept
    {
        if (dists[to][from] < -dist)
            return utils::FALSE_lit; // the constraint is inconsistent
        if (dists[from][to] <= dist)
            return utils::TRUE_lit; // the constraint is trivially satisfied

        // we need to create a new propositional variable..
        const auto ctr = utils::lit(get_sat().new_var());
        bind(variable(ctr));
        auto constr = std::make_unique<distance_constraint<utils::inf_rational>>(ctr, from, to, dist);
        dist_constrs[{from, to}].emplace_back(*constr);
        var_dists.emplace(variable(ctr), std::move(constr));
        return ctr;
    }
    utils::lit rdl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &min, const utils::inf_rational &max) noexcept { return get_sat().new_conj({new_distance(to, from, -min), new_distance(from, to, max)}); }

    utils::lit rdl_theory::new_lt(const utils::lin &left, const utils::lin &right) noexcept
    {
        assert(left.vars.size() <= 2 && right.vars.size() <= 2);
        utils::lin expr = left - right;
        switch (expr.vars.size())
        {
        case 0:
            return expr.known_term < 0 ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                return new_distance(v->first, 0, utils::inf_rational(expr.known_term, -1));
            }
            else
            {
                expr = expr / v->second;
                return new_distance(0, v->first, utils::inf_rational(-expr.known_term, -1));
            }
        }
        case 2:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v0, v1, utils::inf_rational(expr.known_term, -1));
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v1, v0, utils::inf_rational(-expr.known_term, -1));
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit rdl_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
        assert(left.vars.size() <= 2 && right.vars.size() <= 2);
        utils::lin expr = left - right;
        switch (expr.vars.size())
        {
        case 0:
            return expr.known_term < 0 ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                return new_distance(v->first, 0, utils::inf_rational(expr.known_term));
            }
            else
            {
                expr = expr / v->second;
                return new_distance(0, v->first, utils::inf_rational(-expr.known_term));
            }
        }
        case 2:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v0, v1, utils::inf_rational(expr.known_term));
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v1, v0, utils::inf_rational(-expr.known_term));
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit rdl_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept
    {
        assert(left.vars.size() <= 2 && right.vars.size() <= 2);
        utils::lin expr = left - right;
        switch (expr.vars.size())
        {
        case 0:
            return expr.known_term == 0 ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        {
            const auto v = expr.vars.cbegin();
            expr = expr / v->second;
            const auto d = distance(v->first, 0);
            if (d.first <= expr.known_term && d.second >= expr.known_term)
                return get_sat().new_conj({new_distance(v->first, 0, utils::inf_rational(expr.known_term)), new_distance(0, v->first, utils::inf_rational(-expr.known_term))});
            else
                return utils::FALSE_lit;
        }
        case 2:
        {
            expr = expr / expr.vars.cbegin()->second;
            auto it = expr.vars.cbegin();
            const auto [v0, c0] = *it++;
            const auto [v1, c1] = *it;
            const auto d = distance(v0, v1);
            if (d.first <= expr.known_term && d.second >= expr.known_term)
                return get_sat().new_conj({new_distance(v0, v1, utils::inf_rational(expr.known_term)), new_distance(v1, v0, utils::inf_rational(-expr.known_term))});
            else
                return utils::FALSE_lit;
        }
        default:
            assert(false);
        }
    }
    utils::lit rdl_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
    {
        assert(left.vars.size() <= 2 && right.vars.size() <= 2);
        utils::lin expr = left - right;
        switch (expr.vars.size())
        {
        case 0:
            return expr.known_term < 0 ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                return new_distance(v->first, 0, utils::inf_rational(-expr.known_term));
            }
            else
            {
                expr = expr / v->second;
                return new_distance(0, v->first, utils::inf_rational(expr.known_term));
            }
        }
        case 2:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v0, v1, utils::inf_rational(-expr.known_term));
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v1, v0, utils::inf_rational(expr.known_term));
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit rdl_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
    {
        assert(left.vars.size() <= 2 && right.vars.size() <= 2);
        utils::lin expr = left - right;
        switch (expr.vars.size())
        {
        case 0:
            return expr.known_term < 0 ? utils::TRUE_lit : utils::FALSE_lit;
        case 1:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                return new_distance(v->first, 0, utils::inf_rational(-expr.known_term, -1));
            }
            else
            {
                expr = expr / v->second;
                return new_distance(0, v->first, utils::inf_rational(expr.known_term, -1));
            }
        }
        case 2:
        {
            const auto v = expr.vars.cbegin();
            if (is_negative(v->second))
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v0, v1, utils::inf_rational(-expr.known_term, -1));
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one);
                return new_distance(v1, v0, utils::inf_rational(expr.known_term, -1));
            }
        }
        default:
            assert(false);
        }
    }

    std::pair<utils::inf_rational, utils::inf_rational> rdl_theory::bounds(const utils::lin &l) const noexcept
    {
        switch (l.vars.size())
        {
        case 0:
            return {utils::inf_rational(l.known_term), utils::inf_rational(l.known_term)};
        case 1:
        {
            auto v = l.vars.cbegin();
            return {l.known_term + v->second * lb(v->first), l.known_term + v->second * ub(v->first)};
        }
        case 2:
        {
            const auto v0 = l.vars.cbegin();
            const auto v1 = std::next(v0);
            if (v0->second == 1)
            {
                const auto d = distance(v1->first, v0->first);
                return {l.known_term + d.first, l.known_term + d.second};
            }
            else
            {
                const auto d = distance(v0->first, v1->first);
                return {-l.known_term + d.first, -l.known_term + d.second};
            }
        }
        default:
            assert(false);
        }
    }

    bool rdl_theory::matches(const utils::lin &l0, const utils::lin &l1) const
    {
        if (l0.vars.empty() && l1.vars.empty())
            return l0.known_term == l1.known_term;
        else if (l0.vars.empty() && l1.vars.size() == 1)
        {
            const auto [lb, ub] = bounds(l1);
            return lb <= l0.known_term && ub >= l0.known_term;
        }
        else if (l0.vars.size() == 1 && l1.vars.empty())
        {
            const auto [lb, ub] = bounds(l0);
            return lb <= l1.known_term && ub >= l1.known_term;
        }
        else if (l0.vars.size() == 1 && l1.vars.size() == 1)
        {
            const auto [lb, ub] = distance(l0.vars.cbegin()->first, l1.vars.cbegin()->first);
            const auto kt = l0.known_term - l1.known_term;
            return lb + kt <= 0 && ub + kt >= 0;
        }
        else
            throw std::invalid_argument("not a valid comparison between real difference logic expressions..");
    }

    bool rdl_theory::propagate(const utils::lit &p) noexcept
    {
        assert(cnfl.empty());
        assert(var_dists.count(variable(p)));
        auto &constr = *var_dists.at(variable(p));
        switch (get_sat().value(constr.get_lit()))
        {
        case utils::True: // the constraint is asserted directly
            if (dists[constr.get_to()][constr.get_from()] < -constr.get_dist())
            { // the constraint is inconsistent, we have a conflict..
                cnfl.emplace_back(!constr.get_lit());
                VARIABLE_TYPE c_to = constr.get_from();
                while (c_to != constr.get_to())
                {
                    const auto &c_d = dist_constr.find({preds[constr.get_to()][c_to], c_to})->second.get();
                    switch (get_sat().value(c_d.get_lit()))
                    {
                    case utils::True:
                        cnfl.emplace_back(!c_d.get_lit());
                        break;
                    case utils::False:
                        cnfl.emplace_back(c_d.get_lit());
                        break;
                    }
                    c_to = preds[constr.get_to()][c_to];
                }
                return false;
            }
            else if (dists[constr.get_from()][constr.get_to()] > constr.get_dist())
            { // the constraint is not trivially satisfied
                const auto from_to = std::make_pair(constr.get_from(), constr.get_to());
                if (!layers.empty() && !layers.back().old_constrs.count(from_to))
                {
                    if (const auto &c_dist = dist_constr.find(from_to); c_dist != dist_constr.cend()) // we store the current constraint for backtracking purposes..
                        layers.back().old_constrs.emplace(c_dist->first, c_dist->second);
                    else // we store the absence of a constraint for backtracking purposes..
                        layers.back().old_constrs.emplace(from_to, std::nullopt);
                }
                dist_constr.emplace(from_to, constr);
                propagate(constr.get_from(), constr.get_to(), constr.get_dist());
            }
            break;
        case utils::False: // the constraint is asserted negated (a.k.a. semantic branching)
            if (dists[constr.get_from()][constr.get_to()] <= constr.get_dist())
            { // the constraint is inconsistent, we have a conflict..
                cnfl.emplace_back(constr.get_lit());
                VARIABLE_TYPE c_to = constr.get_to();
                while (c_to != constr.get_from())
                {
                    const auto &c_d = dist_constr.find({preds[constr.get_from()][c_to], c_to})->second.get();
                    switch (get_sat().value(c_d.get_lit()))
                    {
                    case utils::True:
                        cnfl.emplace_back(!c_d.get_lit());
                        break;
                    case utils::False:
                        cnfl.emplace_back(c_d.get_lit());
                        break;
                    }
                    c_to = preds[constr.get_from()][c_to];
                }
                return false;
            }
            else if (dists[constr.get_to()][constr.get_from()] >= -constr.get_dist())
            { // the constraint is not trivially satisfied
                const auto to_from = std::make_pair(constr.get_to(), constr.get_from());
                if (!layers.empty() && !layers.back().old_constrs.count(to_from))
                {
                    if (const auto &c_dist = dist_constr.find(to_from); c_dist != dist_constr.cend()) // we store the current constraint for backtracking purposes..
                        layers.back().old_constrs.emplace(c_dist->first, c_dist->second);
                    else // we store the absence of a constraint for backtracking purposes..
                        layers.back().old_constrs.emplace(to_from, std::nullopt);
                }
                dist_constr.emplace(to_from, constr);
                propagate(constr.get_to(), constr.get_from(), -constr.get_dist() - utils::inf_rational::epsilon);
            }
        }
        return true;
    }

    void rdl_theory::propagate(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept
    {
        assert(!is_infinite(dist));
        set_dist(from, to, dist);
        set_pred(from, to, from);
        std::vector<VARIABLE_TYPE> set_i;
        std::vector<VARIABLE_TYPE> set_j;
        std::vector<std::pair<VARIABLE_TYPE, VARIABLE_TYPE>> c_updates;
        c_updates.emplace_back(from, to);
        c_updates.emplace_back(to, from);

        // we start with an O(n) loop..
        for (size_t u = 0; u < n_vars; ++u)
        {
            if (dists[u][from] < dists[u][to] - dist)
            { // u -> from -> to is shorter than u -> to..
                set_dist(u, to, dists[u][from] + dist);
                set_pred(u, to, from);
                set_i.emplace_back(u);
                c_updates.emplace_back(u, to);
                c_updates.emplace_back(to, u);
            }
            if (dists[to][u] < dists[from][u] - dist)
            { // from -> to -> u is shorter than from -> u..
                set_dist(from, u, dists[to][u] + dist);
                set_pred(from, u, preds[to][u]);
                set_j.emplace_back(u);
                c_updates.emplace_back(from, u);
                c_updates.emplace_back(u, from);
            }
        }

        // finally, we loop over set_i and set_j in O(n^2) time (but possibly much less)..
        for (const auto &i : set_i)
            for (const auto &j : set_j)
                if (i != j && dists[i][to] + dists[to][j] < dists[i][j])
                { // i -> from -> to -> j is shorter than i -> j--
                    set_dist(i, j, dists[i][to] + dists[to][j]);
                    set_pred(i, j, preds[to][j]);
                    c_updates.emplace_back(i, j);
                    c_updates.emplace_back(j, i);
                }

        for (const auto &c_pairs : c_updates)
            if (const auto &c_dists = dist_constrs.find(c_pairs); c_dists != dist_constrs.cend())
                for (const auto &c_dist : c_dists->second)
                    if (get_sat().value(c_dist.get().get_lit()) == utils::Undefined)
                    {
                        if (dists[c_dist.get().get_to()][c_dist.get().get_from()] < -c_dist.get().get_dist())
                        { // the constraint is inconsistent..
                            cnfl.emplace_back(!c_dist.get().get_lit());
                            VARIABLE_TYPE c_to = c_dist.get().get_from();
                            while (c_to != c_dist.get().get_to())
                            {
                                const auto &c_d = dist_constr.find({preds[c_dist.get().get_to()][c_to], c_to})->second.get();
                                switch (get_sat().value(c_d.get_lit()))
                                {
                                case utils::True:
                                    cnfl.emplace_back(!c_d.get_lit());
                                    break;
                                case utils::False:
                                    cnfl.emplace_back(c_d.get_lit());
                                    break;
                                }
                                c_to = preds[c_dist.get().get_to()][c_to];
                            }
                            // we propagate the reason for assigning false to dist->b..
                            record(std::move(cnfl));
                        }
                        else if (dists[c_dist.get().get_from()][c_dist.get().get_to()] <= c_dist.get().get_dist())
                        { // the constraint is redundant..
                            cnfl.emplace_back(c_dist.get().get_lit());
                            VARIABLE_TYPE c_to = c_dist.get().get_to();
                            while (c_to != c_dist.get().get_from())
                            {
                                const auto &c_d = dist_constr.find({preds[c_dist.get().get_from()][c_to], c_to})->second.get();
                                switch (get_sat().value(c_d.get_lit()))
                                {
                                case utils::True:
                                    cnfl.emplace_back(!c_d.get_lit());
                                    break;
                                case utils::False:
                                    cnfl.emplace_back(c_d.get_lit());
                                    break;
                                }
                                c_to = preds[c_dist.get().get_from()][c_to];
                            }
                            // we propagate the reason for assigning true to dist->b..
                            record(std::move(cnfl));
                        }
                    }
    }

    void rdl_theory::set_dist(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &dist) noexcept
    {
        assert(dists[from][to] > dist);                                                 // we should never increase the distance
        if (!layers.empty() && !layers.back().old_dists.count({from, to}))              // we have not updated this distance yet
            layers.back().old_dists.emplace(std::make_pair(from, to), dists[from][to]); // save the old distance
        dists[from][to] = dist;                                                         // set the new distance
        if (from == 0)
            FIRE_ON_VALUE_CHANGED(from);
        if (to == 0)
            FIRE_ON_VALUE_CHANGED(to);
    }

    void rdl_theory::set_pred(VARIABLE_TYPE from, VARIABLE_TYPE to, VARIABLE_TYPE pred) noexcept
    {
        assert(preds[from][to] != pred);                                                // we should never set the same predecessor
        if (!layers.empty() && !layers.back().old_preds.count({from, to}))              // we have not updated this predecessor yet
            layers.back().old_preds.emplace(std::make_pair(from, to), preds[from][to]); // save the old predecessor
        preds[from][to] = pred;                                                         // set the new predecessor
    }

    void rdl_theory::resize(const size_t &size) noexcept
    {
        const size_t c_size = dists.size();
        for (size_t i = 0; i < c_size; ++i)
        {
            dists[i].resize(size, utils::inf_rational(utils::rational::positive_infinite));
            preds[i].resize(size, std::numeric_limits<INT_TYPE>::max());
        }
        dists.resize(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite)));
        preds.resize(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INT_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
            preds[i][i] = i;
        }
    }

#ifdef BUILD_LISTENERS
    void rdl_theory::add_listener(rdl_value_listener &l) noexcept
    {
        l.th = this;
        listeners.insert(&l);
    }
    void rdl_theory::remove_listener(rdl_value_listener &l) noexcept
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