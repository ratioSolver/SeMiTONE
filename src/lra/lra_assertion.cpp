#include <cassert>
#include "lra_assertion.hpp"
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_assertion::lra_assertion(lra_theory &th, const op o, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : th(th), o(o), b(b), x(x), v(v) {}

    bool lra_assertion::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        assert(th.cnfl.empty());
        switch (o)
        {
        case leq: // `x <= v`
            switch (th.sat->value(b))
            {
            case utils::True:       // the assertion must be satisfied..
                if (th.lb(x_i) > v) // .. but it is not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                {
                    th.cnfl.push_back(!b);                                             // either the literal `b` is false ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x_i)].reason); // or the reason for the lower bound of `x_i` is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.lb(x_i) > v) // the assertion is not satisfied.. we propagate information to the sat core: [x_i >= lb(x_i)] -> ![x_i <= v]..
                    th.record({!b, !th.c_bounds[lra_theory::lb_index(x_i)].reason});
                break;
            }
            break;
        case geq: // `x >= v`
            switch (th.sat->value(b))
            {
            case utils::False:       // the assertion must be not satisfied..
                if (th.lb(x_i) >= v) // .. but it is satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                {
                    th.cnfl.push_back(b);                                              // either the literal `b` is true ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x_i)].reason); // or the reason for the lower bound of `x_i` is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.lb(x_i) >= v) // the assertion is satisfied.. we propagate information to the sat core: [x_i >= lb(x_i)] -> [x_i >= v]..
                    th.record({b, !th.c_bounds[lra_theory::lb_index(x_i)].reason});
                break;
            }
        }
        return true;
    }

    bool lra_assertion::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        assert(th.cnfl.empty());
        switch (o)
        {
        case leq: // `x <= v`
            switch (th.sat->value(b))
            {
            case utils::False:       // the assertion must be not satisfied..
                if (th.ub(x_i) <= v) // .. but it is satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                {
                    th.cnfl.push_back(b);                                              // either the literal `b` is true ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x_i)].reason); // or the reason for the upper bound of `x_i` is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.ub(x_i) <= v) // the assertion is satisfied.. we propagate information to the sat core: [x_i <= ub(x_i)] -> [x_i <= v]..
                    th.record({b, !th.c_bounds[lra_theory::ub_index(x_i)].reason});
                break;
            }
            break;
        case geq: // `x >= v`
            switch (th.sat->value(b))
            {
            case utils::True:       // the assertion must be satisfied..
                if (th.ub(x_i) < v) // .. but it is not satisfied.. we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                {
                    th.cnfl.push_back(!b);                                             // either the literal `b` is false ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x_i)].reason); // or the reason for the upper bound of `x_i` is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.ub(x_i) < v) // the assertion is not satisfied.. we propagate information to the sat core: [x_i <= ub(x_i)] -> ![x_i >= v]..
                    th.record({!b, !th.c_bounds[lra_theory::ub_index(x_i)].reason});
                break;
            }
        }
        return true;
    }
} // namespace semitone