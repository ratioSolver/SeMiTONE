#include "constr.hpp"
#include "sat_core.hpp"

namespace semitone
{
    utils::lbool constr::value(const VARIABLE_TYPE &x) const noexcept { return sat.value(x); }
    utils::lbool constr::value(const lit &p) const noexcept { return sat.value(p); }

    void constr::remove_constr_from_reason(const VARIABLE_TYPE &x) noexcept
    {
        if (sat.reason[x] == this)
            sat.reason[x] = nullptr;
    }
} // namespace semitone
