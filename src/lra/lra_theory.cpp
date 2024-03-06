#include <cassert>
#include "lra_theory.hpp"
#include "sat_core.hpp"
#include "lra_assertion.hpp"
#include "lra_eq.hpp"
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
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }
    VARIABLE_TYPE lra_theory::new_var(const utils::lin &&l) noexcept
    {
        assert(sat->root_level());
        const auto slack = new_var();
        c_bounds[lb_index(slack)] = {lb(l), utils::TRUE_lit}; // we set the lower bound of the slack variable to the lower bound of the linear expression
        c_bounds[ub_index(slack)] = {ub(l), utils::TRUE_lit}; // we set the upper bound of the slack variable to the upper bound of the linear expression
        vals[slack] = value(l);                               // we set the value of the slack variable to the value of the linear expression
        new_row(slack, std::move(l));                         // we add the new row `slack = ...` to the tableau
        return slack;
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

    void lra_theory::new_row(const VARIABLE_TYPE x_i, const utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend());
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, std::make_unique<lra_eq>(x_i, std::move(xpr)));
    }
} // namespace semitone