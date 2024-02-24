#include <cassert>
#include <algorithm>
#include "sat_disj.hpp"
#include "sat_core.hpp"

namespace semitone
{
    sat_disj::sat_disj(sat_core &s, std::vector<lit> &&ls, const lit &ctr) : constr(s), lits(ls), ctr(ctr)
    {
        assert(s.root_level());
        assert(std::all_of(ls.begin(), ls.end(), [&](const auto &l)
                           { return value(l) == utils::Undefined; }));
        assert(value(ctr) == utils::Undefined);
        for (const auto &l : ls)
        {
            watches(l).push_back(*this);  // any literal that becomes `true` makes the disjunction `true`
            watches(!l).push_back(*this); // any literal that becomes `false` makes the only unassigned literal `true`, if the control variable is `true`
        }
        watches(ctr).push_back(*this);  // making the control variable `true` makes the only unassigned literal `true`, if all the other literals are `false`
        watches(!ctr).push_back(*this); // making the control variable `false` makes all the literals `false`
    }

    std::unique_ptr<constr> sat_disj::copy(sat_core &s) noexcept { return std::make_unique<sat_disj>(s, std::vector<lit>(lits), ctr); }

    bool sat_disj::propagate(const lit &p) noexcept
    {
        assert(value(p) == utils::True);
        watches(p).emplace_back(*this);
        if (!must_propagate(p))
            return true; // the literal is already propagated by this constraint
        if (variable(p) == variable(ctr))
        { // the control variable is assigned
            if (value(ctr) == utils::True)
            { // the disjunction is `true`
                lit u_p;
                bool found = false;
                for (const auto &l : lits)
                    switch (value(l))
                    {
                    case utils::True: // the disjunction is already `true`, nothing to propagate..
                        return true;
                    case utils::Undefined:
                        if (!found)
                        {
                            u_p = l;
                            found = true;
                        }
                        else // more than one literal is unassigned, nothing to propagate..
                            return true;
                    }
                assert(found);
                return enqueue(u_p);
            }
            else
            { // the disjunction is `false`
                assert(value(ctr) == utils::False);
                for (const auto &l : lits)
                    if (!enqueue(!l))
                        return false;
                return true;
            }
        }
        else
        { // a literal is assigned
            assert(std::any_of(lits.begin(), lits.end(), [&p](const auto &l)
                               { return variable(l) == variable(p); })); // the literal must be in the disjunction
            lit u_p;
            bool found = false;
            for (const auto &l : lits)
                switch (value(l))
                {
                case utils::True: // the disjunction must be `true`..
                    return enqueue(ctr);
                case utils::Undefined:
                    if (!found)
                    {
                        u_p = l;
                        found = true;
                    }
                    else // more than one literal is unassigned, nothing to propagate..
                        return true;
                }
            if (value(ctr) == utils::True)
            { // the disjunction is `true` and there is only one unassigned literal
                assert(found);
                return enqueue(u_p); // we have to make the only unassigned literal `true`
            }
            else
            { // all the literals are `false`
                assert(std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                                   { return value(l) == utils::False; }));
                return enqueue(!ctr); // we have to make the disjunction `false`
            }
        }
    }

    bool sat_disj::simplify() noexcept { return value(ctr) != utils::Undefined; }

    std::vector<lit> sat_disj::get_reason(const lit &p) const noexcept
    {
        if (is_undefined(p))
        {
            std::vector<lit> reason;
            reason.reserve(lits.size() + 1);
            for (const auto &l : lits)
                if (value(l) != utils::Undefined)
                    reason.push_back(l);
            if (value(ctr) != utils::Undefined)
                reason.push_back(ctr);
            return reason;
        }
        if (variable(p) == variable(ctr))
        {
            if (value(ctr) == utils::True) // the control variable is `true` because at least one literal is `true`
                return {*std::find_if(lits.begin(), lits.end(), [&](const auto &l)
                                      { return value(l) == utils::True; })};
            else
            { // the control variable is `false` because all the literals are `false`
                assert(std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                                   { return value(l) == utils::False; }));
                return lits;
            }
        }
        else if (value(*std::find_if(lits.begin(), lits.end(), [&](const auto &l)
                                     { return variable(l) == variable(p); })) == utils::True)
        { // the literal is `true`
            assert(std::count_if(lits.begin(), lits.end(), [&](const auto &l)
                                 { return value(l) == utils::True; }) == 1);
            assert(static_cast<size_t>(std::count_if(lits.begin(), lits.end(), [&](const auto &l)
                                                     { return value(l) == utils::False; })) == lits.size() - 1);
            std::vector<lit> reason;
            reason.reserve(lits.size());
            for (const auto &l : lits)
                if (value(l) == utils::False)
                    reason.push_back(l);
                else
                    reason.push_back(ctr);
            return reason;
        }
        else // the literal is `false` because the disjunction is `false`
            return {ctr};
    }

    json::json sat_disj::to_json() const noexcept
    {
        json::json j_conj;
        json::json j_lits(json::json_type::array);
        for (const auto &l : lits)
        {
            json::json j_lit{{"lit", to_string(l)}};
            switch (value(l))
            {
            case utils::True:
                j_lit["value"] = "true";
                break;
            case utils::False:
                j_lit["value"] = "false";
                break;
            }
            j_lits.push_back(std::move(j_lit));
        }
        j_conj["lits"] = std::move(j_lits);
        json::json j_ctr{{"lit", to_string(ctr)}};
        switch (value(ctr))
        {
        case utils::True:
            j_ctr["value"] = "true";
            break;
        case utils::False:
            j_ctr["value"] = "false";
            break;
        }
        j_conj["ctr"] = std::move(j_ctr);
        return j_conj;
    }
} // namespace semitone