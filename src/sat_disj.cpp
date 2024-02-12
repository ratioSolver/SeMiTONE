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
        if (variable(p) == variable(ctr))
            switch (value(variable(ctr)))
            {
            case utils::True:
            {
                lit u_p;
                bool found = false;
                for (const auto &l : lits)
                    if (value(l) == utils::Undefined)
                    {
                        if (!found)
                        {
                            u_p = l;
                            found = true;
                        }
                        else // nothing to propagate..
                            return true;
                    }
                return enqueue(!u_p);
            }
            case utils::False:
                for (const auto &l : lits)
                    if (!enqueue(!l))
                        return false;
                return true;
            }
        else
        {
            assert(std::any_of(lits.begin(), lits.end(), [&p](const auto &l)
                               { return variable(l) == variable(p); }));
            if (value(ctr) == utils::True)
            {
                lit u_p;
                bool found = false;
                for (const auto &l : lits)
                    if (value(l) == utils::Undefined)
                    {
                        if (!found)
                        {
                            u_p = l;
                            found = true;
                        }
                        else // nothing to propagate..
                            return true;
                    }
                return !found || enqueue(u_p);
            }
        }
        return true;
    }
    bool sat_disj::simplify() noexcept
    {
        return std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                           { return value(l) == utils::False; }) ||
               std::any_of(lits.begin(), lits.end(), [&](const auto &l)
                           { return value(l) == utils::True; });
    }

    std::vector<lit> sat_disj::get_reason(const lit &p) const noexcept
    {
        assert(std::all_of(lits.begin(), lits.end(), [&](const auto &l)
                           { return value(l) == utils::False; }) ||
               value(ctr) == utils::True);
        assert(std::any_of(lits.begin(), lits.end(), [&](const auto &l)
                           { return value(l) == utils::True; }) ||
               value(ctr) == utils::False);
        if (is_undefined(p))
        {
            std::vector<lit> reason = lits;
            reason.push_back(ctr);
            return reason;
        }
        if (p == ctr)
            if (value(variable(p)) == utils::True)
                return {*std::find_if(lits.begin(), lits.end(), [&](const auto &l)
                                      { return value(l) == utils::True; })};
            else
                return lits;
        else if (value(variable(p)) == utils::False)
            return {ctr};
        else
        {
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
