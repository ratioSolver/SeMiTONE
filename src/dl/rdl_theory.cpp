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
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit rdl_theory::new_distance(VARIABLE_TYPE from, VARIABLE_TYPE to, const utils::inf_rational &min, const utils::inf_rational &max) noexcept { return sat->new_conj({new_distance(to, from, -min), new_distance(from, to, max)}); }

    utils::lit rdl_theory::new_lt(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit rdl_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit rdl_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit rdl_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit rdl_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
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
                return {l.known_term.numerator() + d.first, l.known_term.numerator() + d.second};
            }
            else
            {
                const auto d = distance(v0->first, v1->first);
                return {-l.known_term.numerator() + d.first, -l.known_term.numerator() + d.second};
            }
        }
        default:
            throw std::invalid_argument("rdl_theory::bounds: invalid linear expression");
        }
    }
} // namespace semitone