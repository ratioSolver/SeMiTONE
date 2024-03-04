#include "lra_eq.hpp"

namespace semitone
{
    lra_eq::lra_eq(const VARIABLE_TYPE x, const utils::lin &&l) noexcept : x(x), l(l) {}

    bool lra_eq::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        return true;
    }

    bool lra_eq::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        return true;
    }
} // namespace semitone