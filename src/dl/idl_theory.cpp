#include <cassert>
#include <stdexcept>
#include "idl_theory.hpp"
#include "integer.hpp"
#include "logging.hpp"

namespace semitone
{
    idl_theory::idl_theory(std::shared_ptr<sat_core> sat, const size_t &size) : theory(sat), dists(size, std::vector<INTEGER_TYPE>(size, utils::inf())), preds(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()))
    {
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = 0;
            preds[i][i] = i;
        }
    }

    VARIABLE_TYPE idl_theory::new_var() noexcept
    {
        auto var = n_vars++;
        if (var >= dists.size())
            resize((dists.size() * 3) / 2 + 1);
        return var;
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

    std::pair<VARIABLE_TYPE, VARIABLE_TYPE> idl_theory::bounds(const utils::lin &l) const
    {
        switch (l.vars.size())
        {
        case 0:
            assert(is_integer(l.known_term));
            return {l.known_term.numerator(), l.known_term.numerator()};
        case 1:
        {
            auto v = l.vars.cbegin();
            assert(is_integer(v->second) && is_integer(l.known_term));
            return {l.known_term.numerator() + v->second.numerator() * lb(v->first), l.known_term.numerator() + v->second.numerator() * ub(v->first)};
        }
        case 2:
        {
            LOG_ERR("idl_theory::bounds not implemented");
        }
        default:
            throw std::invalid_argument("idl_theory::bounds: invalid linear expression");
        }
    }
} // namespace semitone