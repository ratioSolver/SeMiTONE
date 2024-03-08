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
        if (is_positive(l.vars.at(x_i)))
        { // we compute the lower bound of the linear expression along with its reason..
            if (auto c_lb = lb(); !is_infinite(c_lb) && c_lb >= th.lb(x))
                for (const auto &c : th.a_watches[x])
                    if (!c.get().propagate_lb(c_lb))
                        return false;
        }
        else
        { // we compute the upper bound of the linear expression along with its reason..
            if (auto c_ub = ub(); !is_infinite(c_ub) && c_ub <= th.ub(x))
                for (const auto &c : th.a_watches[x])
                    if (!c.get().propagate_ub(c_ub))
                        return false;
        }
        th.cnfl.clear();
        return true;
    }

    bool lra_eq::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        assert(th.cnfl.empty());
        assert(l.vars.find(x_i) != l.vars.end());
        if (is_positive(l.vars.at(x_i)))
        { // we compute the upper bound of the linear expression along with its reason..
            if (auto c_ub = ub(); !is_infinite(c_ub) && c_ub <= th.ub(x))
                for (const auto &c : th.a_watches[x])
                    if (!c.get().propagate_ub(c_ub))
                        return false;
        }
        else
        { // we compute the lower bound of the linear expression along with its reason..
            if (auto c_lb = lb(); !is_infinite(c_lb) && c_lb >= th.lb(x))
                for (const auto &c : th.a_watches[x])
                    if (!c.get().propagate_lb(c_lb))
                        return false;
        }
        th.cnfl.clear();
        return true;
    }

    utils::inf_rational lra_eq::lb() const noexcept
    {
        utils::inf_rational lb(l.known_term);
        th.cnfl.reserve(l.vars.size() + 1);
        // we make room for the first literal..
        th.cnfl.push_back(utils::lit());
        for (const auto &[c_v, c] : l.vars)
        {
            if (is_positive(c))
            {
                lb += th.lb(c_v) * c;
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason);
            }
            else
            {
                lb += th.ub(c_v) * c;
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason);
            }
            if (is_infinite(lb))
                return lb;
        }
        return lb;
    }
    utils::inf_rational lra_eq::ub() const noexcept
    {
        utils::inf_rational ub(l.known_term);
        th.cnfl.reserve(l.vars.size() + 1);
        // we make room for the first literal..
        th.cnfl.push_back(utils::lit());
        for (const auto &[c_v, c] : l.vars)
        {
            if (is_positive(c))
            {
                ub += th.ub(c_v) * c;
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason);
            }
            else
            {
                ub += th.lb(c_v) * c;
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason);
            }
            if (is_infinite(ub))
                return ub;
        }
        return ub;
    }
} // namespace semitone