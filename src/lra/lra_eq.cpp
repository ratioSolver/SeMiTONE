#include <cassert>
#include "lra_eq.hpp"
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_eq::lra_eq(lra_theory &th, const VARIABLE_TYPE x, const utils::lin &&l) noexcept : th(th), x(x), l(l) {}

    bool lra_eq::propagate_lb(const VARIABLE_TYPE x_i) noexcept
    {
        LOG_ERR("lra_eq::propagate_lb not implemented");
        return true;
    }

    bool lra_eq::propagate_ub(const VARIABLE_TYPE x_i) noexcept
    {
        LOG_ERR("lra_eq::propagate_ub not implemented");
        return true;
    }
} // namespace semitone