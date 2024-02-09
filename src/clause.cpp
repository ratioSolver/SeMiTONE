#include <cassert>
#include <functional>
#include <algorithm>
#include "clause.hpp"

namespace semitone
{
    clause::clause(sat_core &s, std::vector<lit> &&ls) : constr(s), lits(std::move(ls))
    {
        assert(lits.size() >= 2);
        watches(!lits[0]).emplace_back(*this);
        watches(!lits[1]).emplace_back(*this);
    }
    clause::~clause()
    {
        auto &ws0 = watches(!lits[0]);
        ws0.erase(std::find_if(ws0.begin(), ws0.end(), [this](const std::reference_wrapper<constr> &c)
                               { return &c.get() == this; }));
        auto &ws1 = watches(!lits[1]);
        ws1.erase(std::find_if(ws1.begin(), ws1.end(), [this](const std::reference_wrapper<constr> &c)
                               { return &c.get() == this; }));
        for (const auto &l : lits)
            remove_constr_from_reason(variable(l));
    }

    bool clause::propagate(const lit &p) noexcept
    {
        // make sure false literal is lits[1]..
        if (variable(lits[0]) == variable(p))
            std::swap(*(lits.begin()), *(std::next(lits.begin())));
        assert(variable(lits[1]) == variable(p));

        // if 0th watch is true, the clause is already satisfied..
        if (value(lits[0]) == utils::True)
        {
            watches(p).emplace_back(*this);
            return true;
        }

        // we look for a new literal to watch..
        for (size_t i = 1; i < lits.size(); ++i)
            if (value(lits[i]) != utils::False)
            {
                std::swap(*(std::next(lits.begin())), *(std::next(lits.begin(), i)));
                watches(!lits[1]).emplace_back(*this);
                return true;
            }

        // clause is unit under assignment..
        watches(p).emplace_back(*this);
        return enqueue(lits[0]);
    }
} // namespace semitone