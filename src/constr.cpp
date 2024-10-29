#include "constr.hpp"
#include "sat_core.hpp"
#include <algorithm>

namespace semitone
{
    bool constr::enqueue(const utils::lit &p) noexcept { return sat.enqueue(p, *this); }

    utils::lbool constr::value(const VARIABLE_TYPE &x) const noexcept { return sat.value(x); }
    utils::lbool constr::value(const utils::lit &p) const noexcept { return sat.value(p); }

    void constr::watch(const utils::lit &p) noexcept { sat.watches[index(p)].emplace_back(*this); }

    void constr::unwatch(const utils::lit &p) noexcept
    {
        std::remove_if(sat.watches[index(p)].begin(), sat.watches[index(p)].end(), [&](const auto &c)
                       { return &c.get() == this; });
    }

    void constr::remove_constr_from_reason(const VARIABLE_TYPE &x) noexcept
    {
        if (sat.reason[x].has_value() && &sat.reason[x].value().get() == this)
            sat.reason[x].reset();
    }
} // namespace semitone
