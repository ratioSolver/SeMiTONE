#include <cassert>
#include <algorithm>
#include "sat_exact_one.hpp"
#include "sat_core.hpp"

namespace semitone
{
    sat_exact_one::sat_exact_one(sat_core &s, std::vector<utils::lit> &&ls, const utils::lit &ctr) : constr(s), lits(std::move(ls)), ctr(ctr)
    {
        assert(s.root_level());
        assert(std::all_of(ls.begin(), ls.end(), [&](const auto &l)
                           { return value(l) == utils::Undefined; }));
        assert(value(ctr) == utils::Undefined);
        for (const auto &l : ls)
        {
            watches(l).push_back(*this);
            watches(!l).push_back(*this);
        }
        watches(ctr).push_back(*this);
        watches(!ctr).push_back(*this);
    }

    std::unique_ptr<constr> sat_exact_one::copy(sat_core &s) noexcept { return std::make_unique<sat_exact_one>(s, std::vector<utils::lit>(lits), ctr); }

    bool sat_exact_one::propagate(const utils::lit &p) noexcept
    {
        assert(value(p) == utils::True);
        watches(p).emplace_back(*this);
        if (!must_propagate(p))
            return true; // the literal is already propagated by this constraint
        if (variable(p) == variable(ctr))
        { // the control variable is assigned
        }
        else
        { // a literal is assigned
        }
    }
} // namespace semitone
