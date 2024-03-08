#include <cassert>
#include "lra_assertion.hpp"
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "logging.hpp"

namespace semitone
{
    bool lra_leq::propagate_lb(const utils::inf_rational &lb) noexcept
    {
        assert(th.cnfl.empty());
        if (lb > v && th.sat->value(b) != utils::False)
        { // either the literal `b` is false or the (precomputed) reason for the lower bound of `x` is false..
            if (th.cnfl.empty())
            { // called from bounds propagation
                th.cnfl.reserve(2);
                th.cnfl.push_back(!b);
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason);
            }
            else // called from lra_eq propagation
                th.cnfl[0] = !b;
            switch (th.sat->value(b))
            {
            case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                return false;
            case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> ![x <= v]..
                th.record(std::move(th.cnfl));
                break;
            }
        }
        return true;
    }

    bool lra_leq::propagate_ub(const utils::inf_rational &ub) noexcept
    {
        assert(th.cnfl.empty());
        if (ub <= v && th.sat->value(b) != utils::True)
        { // either the literal `b` is true or the (precomputed) reason for the upper bound of `x` is false..
            if (th.cnfl.empty())
            { // called from bounds propagation
                th.cnfl.reserve(2);
                th.cnfl.push_back(b);
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason);
            }
            else // called from lra_eq propagation
                th.cnfl[0] = b;
            switch (th.sat->value(b))
            {
            case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                return false;
            case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> [x <= v]..
                th.record(std::move(th.cnfl));
                break;
            }
        }
        return true;
    }

    bool lra_geq::propagate_lb(const utils::inf_rational &lb) noexcept
    {
        assert(th.cnfl.empty());
        if (lb >= v && th.sat->value(b) != utils::True)
        { // either the literal `b` is true or the (precomputed) reason for the lower bound of `x` is false..
            if (th.cnfl.empty())
            { // called from bounds propagation
                th.cnfl.reserve(2);
                th.cnfl.push_back(b);
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason);
            }
            else // called from lra_eq propagation
                th.cnfl[0] = b;
            switch (th.sat->value(b))
            {
            case utils::False: // the assertion should be not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                return false;
            case utils::Undefined: // we propagate information to the sat core: [x >= lb(x)] -> [x >= v]..
                th.record(std::move(th.cnfl));
                break;
            }
        }
        return true;
    }

    bool lra_geq::propagate_ub(const utils::inf_rational &ub) noexcept
    {
        assert(th.cnfl.empty());
        if (ub < v && th.sat->value(b) != utils::False)
        { // either the literal `b` is false or the (precomputed) reason for the upper bound of `x` is false..
            if (th.cnfl.empty())
            { // called from bounds propagation
                th.cnfl.reserve(2);
                th.cnfl.push_back(!b);
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason);
            }
            else // called from lra_eq propagation
                th.cnfl[0] = !b;
            switch (th.sat->value(b))
            {
            case utils::True: // the assertion should be satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                return false;
            case utils::Undefined: // we propagate information to the sat core: [x <= ub(x)] -> ![x >= v]..
                th.record(std::move(th.cnfl));
                break;
            }
        }
        return true;
    }
} // namespace semitone