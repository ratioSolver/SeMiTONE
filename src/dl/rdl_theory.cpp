#include <cassert>
#include <stdexcept>
#include "sat_core.hpp"
#include "rdl_theory.hpp"
#include "logging.hpp"

namespace semitone
{
    rdl_theory::rdl_theory(std::shared_ptr<sat_core> sat, const size_t &size) noexcept : theory(sat), dists(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite))), preds(size, std::vector<VARIABLE_TYPE>(size))
    {
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
            std::fill(preds[i].begin(), preds[i].end(), std::numeric_limits<VARIABLE_TYPE>::max());
            preds[i][i] = i;
        }
        [[maybe_unused]] const auto origin = new_var();
    }
    rdl_theory::rdl_theory(std::shared_ptr<sat_core> sat, const rdl_theory &orig) noexcept : theory(sat), n_vars(orig.n_vars), dists(orig.dists), preds(orig.preds)
    {
        for (const auto &[var, constr] : orig.var_dists)
            var_dists.emplace(var, std::make_unique<distance_constraint<utils::inf_rational>>(*constr));
        for (const auto &[from_to, constr] : orig.dist_constr)
            dist_constr.emplace(from_to, *var_dists.at(variable(constr.get().get_lit())));
        for (const auto &[from_to, constrs] : orig.dist_constrs)
            for (const auto &constr : constrs)
                dist_constrs[from_to].emplace_back(*var_dists.at(variable(constr.get().get_lit())));
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
        const auto ctr = utils::lit(sat->new_var());
        bind(variable(ctr));
        auto constr = std::make_unique<distance_constraint<utils::inf_rational>>(ctr, from, to, dist);
        dist_constrs[{from, to}].emplace_back(*constr);
        var_dists.emplace(variable(ctr), std::move(constr));
        return ctr;
    }
    utils::lit rdl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &min, const utils::inf_rational &max) noexcept { return sat->new_conj({new_distance(to, from, -min), new_distance(from, to, max)}); }

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
                return sat->new_conj({new_distance(v->first, 0, utils::inf_rational(expr.known_term)), new_distance(0, v->first, utils::inf_rational(-expr.known_term))});
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
                return sat->new_conj({new_distance(v0, v1, utils::inf_rational(expr.known_term)), new_distance(v1, v0, utils::inf_rational(-expr.known_term))});
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

    void rdl_theory::resize(const size_t &size) noexcept
    {
        const size_t c_size = dists.size();
        for (size_t i = 0; i < c_size; ++i)
        {
            dists[i].resize(size, utils::inf_rational(utils::rational::positive_infinite));
            preds[i].resize(size, std::numeric_limits<INTEGER_TYPE>::max());
        }
        dists.resize(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite)));
        preds.resize(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
            preds[i][i] = i;
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
} // namespace semitone