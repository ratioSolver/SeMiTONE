#include "constr.hpp"
#include "sat_core.hpp"

namespace semitone
{
    bool constr::enqueue(const utils::lit &p) noexcept { return sat.enqueue(p, *this); }

    std::vector<std::reference_wrapper<constr>> &constr::watches(const utils::lit &p) noexcept { return sat.watches[index(p)]; }

    utils::lbool constr::value(const VARIABLE_TYPE &x) const noexcept { return sat.value(x); }
    utils::lbool constr::value(const utils::lit &p) const noexcept { return sat.value(p); }

    bool constr::must_propagate(const utils::lit &p) const noexcept { return !sat.reason[variable(p)].has_value() || &sat.reason[variable(p)].value().get() != this; }
} // namespace semitone
