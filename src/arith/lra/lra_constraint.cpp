#include "lra_constraint.h"
#include "lra_theory.h"
#include "sat_core.h"
#include <cassert>

namespace semitone
{
    assertion::assertion(lra_theory &th, const op o, const lit b, const var x, const utils::inf_rational &v) : th(th), o(o), b(b), x(x), v(v) { th.a_watches[x].push_back(this); }

    bool assertion::propagate_lb(const var &x_i) noexcept
    {
        assert(th.cnfl.empty());
        switch (o)
        {
        case leq:
            switch (th.sat->value(b))
            {
            case utils::True:
                if (th.lb(x_i) > v)
                {                                                                      // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                    th.cnfl.push_back(!b);                                             // either the literal `b` is false ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x_i)].reason); // or what asserted the lower bound is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.lb(x_i) > v) // we propagate information to the sat core: [x_i >= lb(x_i)] -> ![x_i <= v]..
                    th.record({!b, !th.c_bounds[lra_theory::lb_index(x_i)].reason});
                break;
            }
            break;
        case geq:
            switch (th.sat->value(b))
            {
            case utils::False:
                if (th.lb(x_i) >= v)
                {                                                                      // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                    th.cnfl.push_back(b);                                              // either the literal `b` is true ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(x_i)].reason); // or what asserted the lower bound is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.lb(x_i) >= v) // we propagate information to the sat core: [x_i >= lb(x_i)] -> [x_i >= v]..
                    th.record({b, !th.c_bounds[lra_theory::lb_index(x_i)].reason});
                break;
            }
            break;
        }

        return true;
    }

    bool assertion::propagate_ub(const var &x_i) noexcept
    {
        assert(th.cnfl.empty());
        switch (o)
        {
        case leq:
            switch (th.sat->value(b))
            {
            case utils::False:
                if (th.ub(x_i) <= v)
                {                                                                      // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                    th.cnfl.push_back(b);                                              // either the literal `b` is true ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x_i)].reason); // or what asserted the upper bound is false..
                    return false;
                }
                break;
            case utils::Undefined:
                if (th.ub(x_i) <= v) // we propagate information to the sat core: [x_i <= ub(x_i)] -> [x_i <= v]..
                    th.record({b, !th.c_bounds[lra_theory::ub_index(x_i)].reason});
                break;
            }
            break;
        case geq:
            switch (th.sat->value(b))
            {
            case utils::True:
                if (th.ub(x_i) < v)
                {                                                                      // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                    th.cnfl.push_back(!b);                                             // either the literal `b` is false ..
                    th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(x_i)].reason); // or what asserted the upper bound is false..
                    return false;
                }
                break;
            case utils::Undefined: // we propagate information to the sat core: [x_i <= ub(x_i)] -> ![x_i >= v]..
                if (th.ub(x_i) < v)
                    th.record({!b, !th.c_bounds[lra_theory::ub_index(x_i)].reason});
                break;
            }
            break;
        }

        return true;
    }

    SEMITONE_EXPORT json::json to_json(const assertion &rhs) noexcept
    {
        json::json j_asrt;
        j_asrt["lit"] = to_string(rhs.b);
        switch (rhs.th.get_sat_core().value(rhs.b))
        {
        case utils::True:
            j_asrt["val"] = "T";
            break;
        case utils::False:
            j_asrt["val"] = "F";
            break;
        case utils::Undefined:
            j_asrt["val"] = "U";
            break;
        }
        j_asrt["constr"] = "x" + std::to_string(rhs.x) + (rhs.o == geq ? " >= " : " <= ") + to_string(rhs.v);
        return j_asrt;
    }

    row::row(lra_theory &th, const var x, lin l) : th(th), x(x), l(l) {}

    bool row::propagate_lb(const var &v) noexcept
    {
        assert(th.cnfl.empty());
        // we make room for the first literal..
        th.cnfl.push_back(lit());
        if (is_positive(l.vars.at(v)))
        { // we compute the lower bound of the linear expression along with its reason..
            utils::inf_rational lb(0);
            for (const auto &[c_v, c] : l.vars)
                if (is_positive(c))
                {
                    if (is_negative_infinite(th.lb(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        lb += c * th.lb(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                    }
                }
                else if (is_negative(c))
                {
                    if (is_positive_infinite(th.ub(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        lb += c * th.ub(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                    }
                }

            if (lb >= th.lb(x))
                for (const auto &c : th.a_watches[x])
                    switch (c->o)
                    {
                    case leq: // the assertion is unsatisfable..
                        switch (th.sat->value(c->b))
                        {
                        case utils::True:
                            if (lb > c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = !c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (lb > c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = !c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    case geq: // the assertion is satisfied..
                        switch (th.sat->value(c->b))
                        {
                        case utils::False:
                            if (lb >= c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (lb >= c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    }
        }
        else
        { // we compute the upper bound of the linear expression along with its reason..
            utils::inf_rational ub(0);
            for (const auto &[c_v, c] : l.vars)
                if (is_positive(c))
                {
                    if (is_positive_infinite(th.ub(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        ub += c * th.ub(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                    }
                }
                else if (is_negative(c))
                {
                    if (is_negative_infinite(th.lb(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        ub += c * th.lb(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                    }
                }

            if (ub <= th.ub(x))
                for (const auto &c : th.a_watches[x])
                    switch (c->o)
                    {
                    case leq: // the assertion is satisfied..
                        switch (th.sat->value(c->b))
                        {
                        case utils::False:
                            if (ub <= c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (ub <= c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    case geq: // the assertion is unsatisfable..
                        switch (th.sat->value(c->b))
                        {
                        case utils::True:
                            if (ub < c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = !c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (ub < c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = !c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    }
        }

        th.cnfl.clear();
        return true;
    }

    bool row::propagate_ub(const var &v) noexcept
    {
        assert(th.cnfl.empty());
        // we make room for the first literal..
        th.cnfl.push_back(lit());
        if (is_positive(l.vars.at(v)))
        { // we compute the upper bound of the linear expression along with its reason..
            utils::inf_rational ub(0);
            for (const auto &[c_v, c] : l.vars)
                if (is_positive(c))
                {
                    if (is_positive_infinite(th.ub(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        ub += c * th.ub(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                    }
                }
                else if (is_negative(c))
                {
                    if (is_negative_infinite(th.lb(v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        ub += c * th.lb(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                    }
                }

            if (ub <= th.ub(x))
                for (const auto &c : th.a_watches[x])
                    switch (c->o)
                    {
                    case leq: // the assertion is satisfied..
                        switch (th.sat->value(c->b))
                        {
                        case utils::False:
                            if (ub <= c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (ub <= c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    case geq: // the assertion is unsatisfable..
                        switch (th.sat->value(c->b))
                        {
                        case utils::True:
                            if (ub < c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = !c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (ub < c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = !c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    }
        }
        else
        { // we compute the lower bound of the linear expression along with its reason..
            utils::inf_rational lb(0);
            for (const auto &[c_v, c] : l.vars)
                if (is_positive(c))
                {
                    if (is_negative_infinite(th.lb(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        lb += c * th.lb(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::lb_index(c_v)].reason);
                    }
                }
                else if (is_negative(c))
                {
                    if (is_positive_infinite(th.ub(c_v)))
                    { // nothing to propagate..
                        th.cnfl.clear();
                        return true;
                    }
                    else
                    {
                        lb += c * th.ub(c_v);
                        th.cnfl.push_back(!th.c_bounds[lra_theory::ub_index(c_v)].reason);
                    }
                }

            if (lb >= th.lb(x))
                for (const auto &c : th.a_watches[x])
                    switch (c->o)
                    {
                    case leq: // the assertion is unsatisfable..
                        switch (th.sat->value(c->b))
                        {
                        case utils::True:
                            if (lb > c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = !c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (lb > c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = !c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    case geq: // the assertion is satisfied..
                        switch (th.sat->value(c->b))
                        {
                        case utils::False:
                            if (lb >= c->v)
                            { // we have a propositional inconsistency (notice that this can happen in case some propositional literal has been assigned but the theory did not propagate yet)..
                                th.cnfl[0] = c->b;
                                return false;
                            }
                            break;
                        case utils::Undefined:
                            if (lb >= c->v)
                            { // we propagate information to the sat core..
                                th.cnfl[0] = c->b;
                                th.record(th.cnfl);
                            }
                            break;
                        }
                        break;
                    }
        }

        th.cnfl.clear();
        return true;
    }

    SEMITONE_EXPORT json::json to_json(const row &rhs) noexcept
    {
        json::json j_row;
        j_row["var"] = "x" + std::to_string(rhs.x);
        j_row["expr"] = to_string(rhs.l);
        return j_row;
    }
} // namespace semitone