#include <algorithm>
#include "theory.hpp"
#include "sat_core.hpp"

namespace semitone
{
    theory::theory(std::shared_ptr<sat_core> sat) : sat(sat) { sat->theories.push_back(*this); }
    theory::~theory()
    {
        sat->theories.erase(std::find_if(sat->theories.begin(), sat->theories.end(), [this](const theory &t)
                                         { return &t == this; }));
    }

    void theory::bind(VARIABLE_TYPE v) noexcept { sat->bind(v, *this); }
    void theory::record(std::vector<utils::lit> &&clause) noexcept { sat->record(std::move(clause)); }
} // namespace semitone