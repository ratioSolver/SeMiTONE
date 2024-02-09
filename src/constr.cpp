#include "constr.hpp"
#include "sat_core.hpp"

namespace semitone
{
    bool constr::enqueue(const lit &p) noexcept { return sat.enqueue(p, *this); }

    std::vector<std::reference_wrapper<constr>> &constr::watches(const lit &p) noexcept { return sat.watches[index(p)]; }

    utils::lbool constr::value(const VARIABLE_TYPE &x) const noexcept { return sat.value(x); }
    utils::lbool constr::value(const lit &p) const noexcept { return sat.value(p); }

    void constr::remove_constr_from_reason(const VARIABLE_TYPE &x) noexcept
    {
        if (auto &r = sat.reason[x]; r && &r.value().get() == this)
            r.reset();
    }
} // namespace semitone
