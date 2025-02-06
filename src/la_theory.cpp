#include "la_theory.hpp"
#include <cassert>

namespace semitone
{
    la_theory::la_theory(network &net) noexcept : theory(net) {}

    utils::var la_theory::new_int(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int.push_back(true);
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_real(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        auto var = vals.size();
        is_int.push_back(false);
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_slack(utils::lin &&xpr) noexcept
    {
        auto var = vals.size();
        is_int.push_back(false);

        utils::inf_rational val(xpr.known_term), lb(xpr.known_term), ub(xpr.known_term);
        std::vector<utils::lit> lb_reason, ub_reason;
        for (const auto &[v, c] : xpr.vars)
        {
            val += c * vals[v];
            lb += (is_positive(c) ? c_bounds[lb_index(v)].value : c_bounds[ub_index(v)].value) * c;
            lb_reason.insert(lb_reason.end(), c_bounds[lb_index(v)].reason.cbegin(), c_bounds[lb_index(v)].reason.cend());
            ub += (is_positive(c) ? c_bounds[ub_index(v)].value : c_bounds[lb_index(v)].value) * c;
            ub_reason.insert(ub_reason.end(), c_bounds[ub_index(v)].reason.cbegin(), c_bounds[ub_index(v)].reason.cend());
        }
        c_bounds.emplace_back(bound{lb, std::move(lb_reason)});
        c_bounds.emplace_back(bound{ub, std::move(ub_reason)});
        vals.push_back(val);
        a_watches.emplace_back();
        t_watches.emplace_back();

        new_row(var, std::move(xpr));

        return var;
    }

    void la_theory::propagate(const utils::lit &p) noexcept {}

    void la_theory::new_row(const utils::var x_i, utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, new la_eq(x_i, std::move(xpr)));
    }
} // namespace semitone