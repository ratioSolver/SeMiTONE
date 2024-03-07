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
        switch (th.sat->value(b))
        {
        case utils::True: // the assertion must be satisfied..
            if (lb > v)   // .. but it is not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
            {
                th.cnfl.push_back(!b);                                           // either the literal `b` is false ..
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason); // or the reason for the lower bound of `x` is false..
                return false;
            }
            break;
        case utils::Undefined:
            if (lb > v) // the assertion is not satisfied.. we propagate information to the sat core: [x >= lb(x)] -> ![x <= v]..
                th.record({!b, !th.c_bounds[lra_theory::lb_index(x)].reason});
            break;
        }
        return true;
    }

    bool lra_leq::propagate_ub(const utils::inf_rational &ub) noexcept
    {
        assert(th.cnfl.empty());
        switch (th.sat->value(b))
        {
        case utils::False: // the assertion must be not satisfied..
            if (ub <= v)   // .. but it is satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
            {
                th.cnfl.push_back(b);                                            // either the literal `b` is true ..
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason); // or the reason for the upper bound of `x` is false..
                return false;
            }
            break;
        case utils::Undefined:
            if (ub <= v) // the assertion is satisfied.. we propagate information to the sat core: [x <= ub(x)] -> [x <= v]..
                th.record({b, !th.c_bounds[lra_theory::ub_index(x)].reason});
            break;
        }
        return true;
    }

    bool lra_geq::propagate_lb(const utils::inf_rational &lb) noexcept
    {
        assert(th.cnfl.empty());
        switch (th.sat->value(b))
        {
        case utils::False: // the assertion must be not satisfied..
            if (lb >= v)   // .. but it is satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
            {
                th.cnfl.push_back(b);                                            // either the literal `b` is true ..
                th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x)].reason); // or the reason for the lower bound of `x` is false..
                return false;
            }
            break;
        case utils::Undefined:
            if (lb >= v) // the assertion is satisfied.. we propagate information to the sat core: [x >= lb(x)] -> [x >= v]..
                th.record({b, !th.c_bounds[lra_theory::lb_index(x)].reason});
            break;
        }
        return true;
    }

    bool lra_geq::propagate_ub(const utils::inf_rational &ub) noexcept
    {
        assert(th.cnfl.empty());
        switch (th.sat->value(b))
        {
        case utils::True: // the assertion must be satisfied..
            if (ub < v)   // .. but it is not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
            {
                th.cnfl.push_back(!b);                                           // either the literal `b` is false ..
                th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x)].reason); // or the reason for the upper bound of `x` is false..
                return false;
            }
            break;
        case utils::Undefined:
            if (ub < v) // the assertion is not satisfied.. we propagate information to the sat core: [x <= ub(x)] -> ![x >= v]..
                th.record({!b, !th.c_bounds[lra_theory::ub_index(x)].reason});
            break;
        }
        return true;
    }
} // namespace semitone