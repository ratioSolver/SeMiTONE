#include <cassert>
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_eq::lra_eq(lra_theory &th, const VARIABLE_TYPE x, const utils::lin &&l) noexcept : th(th), x(x), l(l) {}

    bool lra_eq::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        assert(th.cnfl.empty());
        assert(l.vars.find(x_i) != l.vars.end());
        // we make room for the first literal..
        th.cnfl.push_back(utils::lit());
        if (is_positive(l.vars.at(x_i)))
        { // we compute the lower bound of the linear expression along with its reason..
            if (auto var_lb = free_var_lb(); var_lb.has_value())
                for (const auto &c : th.a_watches[var_lb->first])
                    if (!c.get().propagate_lb(var_lb->second))
                        return false;
        }
        else
        { // we compute the upper bound of the linear expression along with its reason..
            if (auto var_ub = free_var_ub(); var_ub.has_value())
                for (const auto &c : th.a_watches[var_ub->first])
                    if (!c.get().propagate_ub(var_ub->second))
                        return false;
        }
        th.cnfl.clear();
        return true;
    }

    bool lra_eq::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        assert(th.cnfl.empty());
        assert(l.vars.find(x_i) != l.vars.end());
        // we make room for the first literal..
        th.cnfl.push_back(utils::lit());
        if (is_positive(l.vars.at(x_i)))
        { // we compute the upper bound of the linear expression along with its reason..
            if (auto var_ub = free_var_ub(); var_ub.has_value())
                for (const auto &c : th.a_watches[var_ub->first])
                    if (!c.get().propagate_ub(var_ub->second))
                        return false;
        }
        else
        { // we compute the lower bound of the linear expression along with its reason..
            if (auto var_lb = free_var_lb(); var_lb.has_value())
                for (const auto &c : th.a_watches[var_lb->first])
                    if (!c.get().propagate_lb(var_lb->second))
                        return false;
        }
        th.cnfl.clear();
        return true;
    }

    [[nodiscard]] std::optional<std::pair<VARIABLE_TYPE, utils::inf_rational>> lra_eq::free_var_lb() const noexcept
    {
        utils::lin l_expr = l - utils::lin(x, utils::rational(1));
        utils::inf_rational lb(l.known_term);
        VARIABLE_TYPE free_var = std::numeric_limits<VARIABLE_TYPE>::max();
        for (const auto &[c_v, c] : l_expr.vars)
            if (is_positive(c))
            {
                if (auto c_lb = th.lb(c_v); !is_infinite(c_lb))
                { // `c_v` has a lower bound that might be useful for the propagation..
                    lb += c_lb * c;
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                } // we have a free variable..
                else if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
                    free_var = c_v;
                else // we have more than one free variable..
                    return std::nullopt;
            }
            else
            {
                if (auto c_ub = th.ub(c_v); !is_infinite(c_ub))
                { // `c_v` has an upper bound that might be useful for the propagation..
                    lb += c_ub * c;
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                } // we have a free variable..
                else if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
                    free_var = c_v;
                else // we have more than one free variable..
                    return std::nullopt;
            }
        if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
            return std::nullopt;
        return std::make_pair(free_var, lb);
    }
    [[nodiscard]] std::optional<std::pair<VARIABLE_TYPE, utils::inf_rational>> lra_eq::free_var_ub() const noexcept
    {
        utils::lin l_expr = l - utils::lin(x, utils::rational(1));
        utils::inf_rational ub(l.known_term);
        VARIABLE_TYPE free_var = std::numeric_limits<VARIABLE_TYPE>::max();
        for (const auto &[c_v, c] : l_expr.vars)
            if (is_positive(c))
            {
                if (auto c_ub = th.ub(c_v); !is_infinite(c_ub))
                { // `c_v` has an upper bound that might be useful for the propagation..
                    ub += c_ub * c;
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                } // we have a free variable..
                else if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
                    free_var = c_v;
                else // we have more than one free variable..
                    return std::nullopt;
            }
            else
            {
                if (auto c_lb = th.lb(c_v); !is_infinite(c_lb))
                { // `c_v` has a lower bound that might be useful for the propagation..
                    ub += c_lb * c;
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                } // we have a free variable..
                else if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
                    free_var = c_v;
                else // we have more than one free variable..
                    return std::nullopt;
            }
        if (free_var == std::numeric_limits<VARIABLE_TYPE>::max())
            return std::nullopt;
        return std::make_pair(free_var, ub);
    }
} // namespace semitone