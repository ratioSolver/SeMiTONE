#include "theory.hpp"
#include "sat_core.hpp"

namespace semitone
{
    void theory::bind(VARIABLE_TYPE v) noexcept { sat->bind(v, *this); }
} // namespace semitone