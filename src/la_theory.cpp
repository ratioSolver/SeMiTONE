#include "la_theory.hpp"

namespace semitone
{
    la_theory::la_theory(network &net) noexcept : theory(net) {}

    void la_theory::propagate(const utils::lit &p) noexcept {}
} // namespace semitone