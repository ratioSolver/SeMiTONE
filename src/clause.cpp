#include "clause.h"
#include <cassert>

namespace semitone
{
    clause::clause(sat_core &s, std::vector<lit> lits) : constr(s), lits(std::move(lits)) {}
} // namespace smt