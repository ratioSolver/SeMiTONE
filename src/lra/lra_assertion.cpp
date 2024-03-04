#include "lra_assertion.hpp"

namespace semitone
{
    lra_assertion::lra_assertion(const op o, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : o(o), b(b), x(x), v(v) {}

    bool lra_assertion::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        return true;
    }

    bool lra_assertion::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        return true;
    }
} // namespace semitone