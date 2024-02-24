#include "lra_theory.hpp"

namespace semitone
{
    lra_theory::lra_theory(std::shared_ptr<sat_core> sat) : theory(sat) {}
} // namespace semitone