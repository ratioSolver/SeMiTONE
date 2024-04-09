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
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit idl_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit idl_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit idl_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
    }
    utils::lit idl_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
    {
        throw std::runtime_error("Not implemented yet");
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

    std::pair<VARIABLE_TYPE, VARIABLE_TYPE> idl_theory::bounds(const utils::lin &l) const noexcept
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
            throw std::invalid_argument("idl_theory::bounds: invalid linear expression");
        }
    }
} // namespace semitone