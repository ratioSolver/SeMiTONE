#include "lra_assertion.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_assertion::lra_assertion(const op o, const utils::lit b, const VARIABLE_TYPE x, const utils::inf_rational &v) noexcept : o(o), b(b), x(x), v(v) {}

    bool lra_assertion::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        LOG_ERR("lra_assertion::propagate_lb not implemented");
        return true;
    }

    bool lra_assertion::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        LOG_ERR("lra_assertion::propagate_ub not implemented");
        return true;
    }
} // namespace semitone