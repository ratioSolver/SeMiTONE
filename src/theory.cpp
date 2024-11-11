#include "theory.hpp"
#include "sat_core.hpp"
#include "clause.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cassert>

namespace semitone
{
    void theory::bind(std::size_t v) noexcept { sat->bind(v, *this); }

    bool theory::backtrack_analyze_and_backjump() noexcept
    {
        size_t bt_level = 0;
        if (cnfl.size() == 1)
        { // we can directly enqueue the literal..
            while (sat->decision_level() > 0)
                sat->pop();
            if (!sat->enqueue(cnfl[0]))
                return false;
            return sat->propagate();
        }

        // we backtrack to a level at which we can analyze the conflict..
        for (const auto &l : cnfl)
            if (bt_level < sat->level[variable(l)])
                bt_level = sat->level[variable(l)];

        while (sat->decision_level() > bt_level)
            sat->pop();

        if (sat->root_level())
            return sat->new_clause(std::move(cnfl)) && sat->propagate();

        // we analyze the conflict and backjump..
        analyze_and_backjump();
        return sat->propagate();
    }

    void theory::analyze_and_backjump() noexcept
    {
        // we create a conflict clause for the analysis..
        clause cnfl_cl(*sat, std::move(cnfl));

        // .. and we analyze the conflict..
        std::vector<utils::lit> no_good;
        size_t bt_level = 0;
        sat->analyze(cnfl_cl, no_good, bt_level);

        // we backjump..
        while (sat->decision_level() > bt_level)
            sat->pop();
        // .. and record the no-good..
        sat->record(no_good);
    }

    void theory::set_theory_conflict(std::vector<utils::lit> &&c) noexcept
    {
        cnfl = std::move(c);
        std::sort(cnfl.begin(), cnfl.end());
        utils::lit p;
        size_t j = 0;
        for (auto it = cnfl.cbegin(); it != cnfl.cend(); ++it)
            if (*it != p)
            { // we include this literal in the clause..
                p = *it;
                cnfl[j++] = p;
            }
        cnfl.resize(j);
    }

    void theory::record(std::vector<utils::lit> &&clause) noexcept { sat->record(std::move(clause)); }
} // namespace semitone