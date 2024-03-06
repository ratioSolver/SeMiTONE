#include <cassert>
#include <stdexcept>
#include "rdl_theory.hpp"
#include "integer.hpp"

namespace semitone
{
    rdl_theory::rdl_theory(std::shared_ptr<sat_core> sat, const size_t &size) : theory(sat), dists(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite))), preds(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()))
    {
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
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

    std::pair<utils::inf_rational, utils::inf_rational> rdl_theory::bounds(const utils::lin &l) const
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
            auto v = l.vars.cbegin();
            auto w = std::next(v);
            return {l.known_term + v->second * lb(v->first) + w->second * lb(w->first), l.known_term + v->second * ub(v->first) + w->second * ub(w->first)};
        }
        default:
            throw std::runtime_error("rdl_theory::bounds not implemented");
        }
    }
} // namespace semitone