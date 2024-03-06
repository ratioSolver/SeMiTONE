#include "lra_theory.hpp"
#include "logging.hpp"

namespace semitone
{
    lra_theory::lra_theory(std::shared_ptr<sat_core> sat) : theory(sat) {}

    VARIABLE_TYPE lra_theory::new_var() noexcept
    {
        auto var = vals.size();
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::negative_infinite), utils::TRUE_lit});
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::positive_infinite), utils::TRUE_lit});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        return var;
    }

    utils::lit lra_theory::new_lt(const utils::lin &left, const utils::lin &right) noexcept
    {
        LOG_ERR("lra_theory::new_lt not implemented");
        return utils::TRUE_lit;
    }
    utils::lit lra_theory::new_leq(const utils::lin &left, const utils::lin &right) noexcept
    {
        LOG_ERR("lra_theory::new_leq not implemented");
        return utils::TRUE_lit;
    }
    utils::lit lra_theory::new_eq(const utils::lin &left, const utils::lin &right) noexcept
    {
        LOG_ERR("lra_theory::new_eq not implemented");
        return utils::TRUE_lit;
    }
    utils::lit lra_theory::new_geq(const utils::lin &left, const utils::lin &right) noexcept
    {
        LOG_ERR("lra_theory::new_geq not implemented");
        return utils::TRUE_lit;
    }
    utils::lit lra_theory::new_gt(const utils::lin &left, const utils::lin &right) noexcept
    {
        LOG_ERR("lra_theory::new_gt not implemented");
        return utils::TRUE_lit;
    }
} // namespace semitone