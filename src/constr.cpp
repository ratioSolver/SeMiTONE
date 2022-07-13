#include "constr.h"
#include "sat_core.h"

namespace semitone
{
    constr::constr(sat_core &s) : sat(s), id(s.constrs.size()) {}
} // namespace semitone