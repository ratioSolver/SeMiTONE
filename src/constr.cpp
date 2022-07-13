#include "constr.h"
#include "sat_core.h"

namespace semitone
{
    constr::constr(sat_core &s) : sat(s), id(s.constrs.size()) {}

    std::vector<constr *> &constr::watches(const lit &p) noexcept { return sat.watches[index(p)]; }

    lbool constr::value(const var &x) const noexcept { return sat.value(x); }
    lbool constr::value(const lit &p) const noexcept { return sat.value(p); }
} // namespace semitone