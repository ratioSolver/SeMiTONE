#include <algorithm>
#include "theory.hpp"
#include "sat_core.hpp"
#include "clause.hpp"

namespace semitone
{
    theory::theory(std::shared_ptr<sat_core> sat) noexcept : sat(sat) { sat->theories.push_back(*this); }
    theory::~theory()
    {
        sat->theories.erase(std::find_if(sat->theories.begin(), sat->theories.end(), [this](const theory &t)
                                         { return &t == this; }));
    }

    void theory::bind(VARIABLE_TYPE v) noexcept { sat->bind(v, *this); }

    void theory::analyze_and_backjump() noexcept
    {
        // we create a conflict clause for the analysis..
        clause cnfl_cl(*sat, std::move(cnfl));

        // .. and we analyze the conflict..
        std::vector<utils::lit> no_good;
        size_t bt_level;
        sat->analyze(cnfl_cl, no_good, bt_level);
        cnfl.clear();

        // we backjump..
        while (sat->decision_level() > bt_level)
            sat->pop();
        // .. and record the no-good..
        sat->record(no_good);
    }

    void theory::record(std::vector<utils::lit> &&clause) noexcept { sat->record(std::move(clause)); }
} // namespace semitone