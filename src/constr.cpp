#include "constr.hpp"
#include "sat_core.hpp"
#include <cassert>
#include <algorithm>

namespace semitone
{
    bool constr::enqueue(const utils::lit &p) noexcept { return sat.enqueue(p, *this); }

    utils::lbool constr::value(const utils::var &x) const noexcept { return sat.value(x); }
    utils::lbool constr::value(const utils::lit &p) const noexcept { return sat.value(p); }

    void constr::watch(const utils::lit &p) noexcept { sat.watches[index(p)].emplace_back(*this); }

    void constr::unwatch(const utils::lit &p) noexcept
    {
        assert(!sat.watches[index(p)].empty());
        assert(std::any_of(sat.watches[index(p)].begin(), sat.watches[index(p)].end(), [&](const auto &w)
                           { return &w.get() == this; }));
        auto &ws = sat.watches[index(p)];
        ws.erase(std::remove_if(ws.begin(), ws.end(), [&](const auto &w)
                                { return &w.get() == this; }),
                 ws.end());
    }

    void constr::remove_constr_from_reason(const utils::var &x) noexcept
    {
        if (sat.reason[x].has_value() && &sat.reason[x]->get() == this)
            sat.reason[x].reset();
    }
} // namespace semitone
