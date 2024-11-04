#include "theory.hpp"
#include "sat_core.hpp"
#include "clause.hpp"
#include "logging.hpp"
#include <algorithm>

namespace semitone
{
    void theory::bind(VARIABLE_TYPE v) noexcept { sat->bind(v, *this); }

    bool theory::backtrack_analyze_and_backjump() noexcept
    {
        // we backtrack to a level at which we can analyze the conflict..
        size_t bt_level = 0;
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
        std::sort(cnfl.begin(), cnfl.end());
        utils::lit p;
        size_t j = 0;
        size_t bt_level = 0;
        for (auto it = cnfl.cbegin(); it != cnfl.cend(); ++it)
            if (*it != p && sat->level[variable(*it)] > 0)
            { // we include this literal in the clause..
                p = *it;
                cnfl[j++] = p;
                if (bt_level < sat->level[variable(p)])
                    bt_level = sat->level[variable(p)];
            }
        cnfl.resize(j);

        // while (sat->decision_level() > bt_level)
        //     sat->pop();

        // we create a conflict clause for the analysis..
        clause cnfl_cl(*sat, std::move(cnfl));

        // .. and we analyze the conflict..
        std::vector<utils::lit> no_good;
        sat->analyze(cnfl_cl, no_good, bt_level);

        // we backjump..
        while (sat->decision_level() > bt_level)
            sat->pop();
        // .. and record the no-good..
        sat->record(no_good);
    }

    void theory::set_theory_conflict(std::vector<utils::lit> &&cnfl) noexcept { this->cnfl = std::move(cnfl); }

    void theory::record(std::vector<utils::lit> &&clause) noexcept { sat->record(std::move(clause)); }
} // namespace semitone