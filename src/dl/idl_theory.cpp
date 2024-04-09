#include <cassert>
#include <stdexcept>
#include "sat_core.hpp"
#include "idl_theory.hpp"
#include "integer.hpp"
#include "logging.hpp"

namespace semitone
{
    idl_theory::idl_theory(std::shared_ptr<sat_core> sat, const size_t &size) noexcept : theory(sat), dists(size, std::vector<INTEGER_TYPE>(size, utils::inf())), preds(size, std::vector<VARIABLE_TYPE>(size))
    {
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = 0;
            std::fill(preds[i].begin(), preds[i].end(), std::numeric_limits<VARIABLE_TYPE>::max());
            preds[i][i] = i;
        }
        [[maybe_unused]] const auto origin = new_var();
    }
    idl_theory::idl_theory(std::shared_ptr<sat_core> sat, const idl_theory &orig) noexcept : theory(sat), n_vars(orig.n_vars), dists(orig.dists), preds(orig.preds)
    {
        for (const auto &[var, constr] : orig.var_dists)
            var_dists.emplace(var, std::make_unique<distance_constraint<INTEGER_TYPE>>(*constr));
        for (const auto &[from_to, constr] : orig.dist_constr)
            dist_constr.emplace(from_to, *var_dists.at(variable(constr.get().get_lit())));
        for (const auto &[from_to, constrs] : orig.dist_constrs)
            for (const auto &constr : constrs)
                dist_constrs[from_to].emplace_back(*var_dists.at(variable(constr.get().get_lit())));
    }

    VARIABLE_TYPE idl_theory::new_var() noexcept
    {
        auto var = n_vars++;
        if (var >= dists.size())
            resize((dists.size() * 3) / 2 + 1);
        return var;
    }

    utils::lit idl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INTEGER_TYPE dist) noexcept
    {
        if (dists[to][from] < -dist)
            return utils::FALSE_lit; // the constraint is inconsistent
        if (dists[from][to] <= dist)
            return utils::TRUE_lit; // the constraint is trivially satisfied

        // we need to create a new propositional variable..
        const auto ctr = utils::lit(sat->new_var());
        bind(variable(ctr));
        auto constr = std::make_unique<distance_constraint<INTEGER_TYPE>>(ctr, from, to, dist);
        dist_constrs[{from, to}].emplace_back(*constr);
        var_dists.emplace(variable(ctr), std::move(constr));
        return ctr;
    }
    utils::lit idl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, INTEGER_TYPE min, INTEGER_TYPE max) noexcept { return sat->new_conj({new_distance(to, from, -min), new_distance(from, to, max)}); }

    utils::lit idl_theory::new_lt(const utils::lin &left, const utils::lin &right) noexcept
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
                assert(is_integer(expr.known_term));
                return new_distance(v->first, 0, expr.known_term.numerator() - 1);
            }
            else
            {
                expr = expr / v->second;
                assert(is_integer(expr.known_term));
                return new_distance(0, v->first, -expr.known_term.numerator() - 1);
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
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v0, v1, expr.known_term.numerator() - 1);
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v1, v0, -expr.known_term.numerator() - 1);
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit idl_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
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
                assert(is_integer(expr.known_term));
                return new_distance(v->first, 0, expr.known_term.numerator());
            }
            else
            {
                expr = expr / v->second;
                assert(is_integer(expr.known_term));
                return new_distance(0, v->first, -expr.known_term.numerator());
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
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v0, v1, expr.known_term.numerator());
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v1, v0, -expr.known_term.numerator());
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit idl_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept
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
            assert(is_integer(expr.known_term));
            const auto d = distance(v->first, 0);
            if (d.first <= expr.known_term.numerator() && d.second >= expr.known_term.numerator())
                return sat->new_conj({new_distance(v->first, 0, expr.known_term.numerator()), new_distance(0, v->first, -expr.known_term.numerator())});
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
            if (d.first <= expr.known_term.numerator() && d.second >= expr.known_term.numerator())
                return sat->new_conj({new_distance(v0, v1, expr.known_term.numerator()), new_distance(v1, v0, -expr.known_term.numerator())});
            else
                return utils::FALSE_lit;
        }
        default:
            assert(false);
        }
    }
    utils::lit idl_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
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
                assert(is_integer(expr.known_term));
                return new_distance(v->first, 0, -expr.known_term.numerator());
            }
            else
            {
                expr = expr / v->second;
                assert(is_integer(expr.known_term));
                return new_distance(0, v->first, expr.known_term.numerator());
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
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v0, v1, -expr.known_term.numerator());
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v1, v0, expr.known_term.numerator());
            }
        }
        default:
            assert(false);
        }
    }
    utils::lit idl_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
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
                assert(is_integer(expr.known_term));
                return new_distance(v->first, 0, -expr.known_term.numerator() - 1);
            }
            else
            {
                expr = expr / v->second;
                assert(is_integer(expr.known_term));
                return new_distance(0, v->first, expr.known_term.numerator() - 1);
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
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v0, v1, -expr.known_term.numerator() - 1);
            }
            else
            {
                expr = expr / v->second;
                auto it = expr.vars.cbegin();
                const auto [v0, c0] = *it++;
                const auto [v1, c1] = *it;
                assert(c0 == utils::rational::one && c1 == -utils::rational::one && is_integer(expr.known_term));
                return new_distance(v1, v0, expr.known_term.numerator() - 1);
            }
        }
        default:
            assert(false);
        }
    }

    std::pair<INTEGER_TYPE, INTEGER_TYPE> idl_theory::bounds(const utils::lin &l) const noexcept
    {
        switch (l.vars.size())
        {
        case 0:
            assert(is_integer(l.known_term));
            return {l.known_term.numerator(), l.known_term.numerator()};
        case 1:
        {
            const auto v = l.vars.cbegin();
            assert(is_integer(v->second) && is_integer(l.known_term));
            return {l.known_term.numerator() + v->second.numerator() * lb(v->first), l.known_term.numerator() + v->second.numerator() * ub(v->first)};
        }
        case 2:
        {
            const auto v0 = l.vars.cbegin();
            const auto v1 = std::next(v0);
            assert(((v0->second == 1 && v1->second == -1) || (v0->second == -1 && v1->second == 1)) && is_integer(l.known_term));
            if (v0->second == 1)
            {
                const auto d = distance(v1->first, v0->first);
                return {l.known_term.numerator() + d.first, l.known_term.numerator() + d.second};
            }
            else
            {
                const auto d = distance(v0->first, v1->first);
                return {-l.known_term.numerator() + d.first, -l.known_term.numerator() + d.second};
            }
        }
        default:
            assert(false);
        }
    }

    void idl_theory::set_dist(VARIABLE_TYPE from, VARIABLE_TYPE to, INTEGER_TYPE dist) noexcept
    {
        assert(dists[from][to] > dist);                                                 // we should never increase the distance
        if (!layers.empty() && !layers.back().old_dists.count({from, to}))              // we have not updated this distance yet
            layers.back().old_dists.emplace(std::make_pair(from, to), dists[from][to]); // save the old distance
        dists[from][to] = dist;                                                         // set the new distance
    }

    void idl_theory::set_pred(VARIABLE_TYPE from, VARIABLE_TYPE to, VARIABLE_TYPE pred) noexcept
    {
        assert(preds[from][to] != pred);                                                // we should never set the same predecessor
        if (!layers.empty() && !layers.back().old_preds.count({from, to}))              // we have not updated this predecessor yet
            layers.back().old_preds.emplace(std::make_pair(from, to), preds[from][to]); // save the old predecessor
        preds[from][to] = pred;                                                         // set the new predecessor
    }

    void idl_theory::resize(const size_t &size) noexcept
    {
        const size_t c_size = dists.size();
        for (size_t i = 0; i < c_size; ++i)
        {
            dists[i].resize(size, utils::inf());
            preds[i].resize(size, std::numeric_limits<INTEGER_TYPE>::max());
        }
        dists.resize(size, std::vector<INTEGER_TYPE>(size, utils::inf()));
        preds.resize(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = 0;
            preds[i][i] = i;
        }
    }
} // namespace semitone