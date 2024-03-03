#include <cassert>
#include <algorithm>
#include "sat_conj.hpp"
#include "sat_core.hpp"

namespace semitone
{
    sat_conj::sat_conj(sat_core &s, std::vector<utils::lit> &&ls, const utils::lit &ctr) : constr(s), lits(ls), ctr(ctr)
    {
        assert(s.root_level());
        assert(std::all_of(ls.begin(), ls.end(), [&](const auto &l)
                           { return value(l) == utils::Undefined; }));
        assert(value(ctr) == utils::Undefined);
        for (const auto &l : ls)
        {
            watches(l).push_back(*this);  // any literal that becomes `true` makes the only unassigned literal `false`, if the control variable is `false`
            watches(!l).push_back(*this); // any literal that becomes `false` makes the conjunction `false`
        }
        watches(ctr).push_back(*this);  // making the control variable `true` makes all the literals `true`
        watches(!ctr).push_back(*this); // making the control variable `false` makes the only unassigned literal `false`, if all the other literals are `true`
    }

    std::unique_ptr<constr> sat_conj::copy(sat_core &s) noexcept { return std::make_unique<sat_conj>(s, std::vector<utils::lit>(lits), ctr); }

    bool sat_conj::propagate(const utils::lit &p) noexcept
    {
        assert(value(p) == utils::True);
        watches(p).emplace_back(*this);
        if (!must_propagate(p))
            return true; // the literal is already propagated by this constraint
        if (variable(p) == variable(ctr))
        { // the control variable is assigned
            if (value(ctr) == utils::True)
            { // the conjunction is `true`
                for (const auto &l : lits)
                    if (!enqueue(l))
                        return false;
                return true;
            }
            else
            { // the conjunction is `false`
                assert(value(ctr) == utils::False);
                utils::lit u_p;
                bool found = false;
                for (const auto &l : lits)
                    switch (value(l))
                    {
                    case utils::False: // the conjunction is already `false`, nothing to propagate..
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
                return enqueue(!u_p);
            }
        }
        else
        { // a literal is assigned
            assert(std::any_of(lits.begin(), lits.end(), [&p](const auto &l)
                               { return variable(l) == variable(p); })); // the literal must be in the conjunction
            utils::lit u_p;
            bool found = false;
            for (const auto &l : lits)
                switch (value(l))
                {
                case utils::False: // the conjunction must be `false`..
                    return enqueue(!ctr);
                case utils::Undefined:
                    if (!found)
                    {
                        u_p = l;
                        found = true;
                    }
                    else // more than one literal is unassigned, nothing to propagate..
                        return true;
                }
            if (value(ctr) == utils::False)
            { // the conjunction is `false` and there is only one unassigned literal
                assert(found);
                return enqueue(!u_p); // we have to make the only unassigned literal `false`
            }
            else
            { // all the literals are `true`
                assert(std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                                   { return value(l) == utils::True; }));
                return enqueue(ctr); // we have to make the conjunction `true`
            }
        }
    }

    bool sat_conj::simplify() noexcept { return value(ctr) != utils::Undefined; }

    std::vector<utils::lit> sat_conj::get_reason(const utils::lit &p) const noexcept
    {
        if (is_undefined(p))
        {
            std::vector<utils::lit> reason;
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
            if (value(ctr) == utils::True)
            { // the control variable is `true` because all the literals are `true`
                assert(std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                                   { return value(l) == utils::True; }));
                return lits;
            }
            else // the control variable is `false` because at least one literal is `false`
                return {*std::find_if(lits.begin(), lits.end(), [&](const auto &l)
                                      { return value(l) == utils::False; })};
        }
        else if (value(*std::find_if(lits.begin(), lits.end(), [&](const auto &l) // the literal is `true`
                                     { return variable(l) == variable(p); })) == utils::True)
            return {ctr}; // the literal is `true` because the conjunction is `true`
        else
        { // the literal is `false`
            assert(std::count_if(lits.begin(), lits.end(), [&](const auto &l)
                                 { return value(l) == utils::False; }) == 1);
            assert(static_cast<size_t>(std::count_if(lits.begin(), lits.end(), [&](const auto &l)
                                                     { return value(l) == utils::True; })) == lits.size() - 1);
            std::vector<utils::lit> reason;
            reason.reserve(lits.size());
            for (const auto &l : lits)
                if (value(l) == utils::True)
                    reason.push_back(l);
                else
                    reason.push_back(ctr);
            return reason;
        }
    }

    json::json sat_conj::to_json() const noexcept
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
