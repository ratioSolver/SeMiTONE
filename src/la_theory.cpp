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

    utils::var la_theory::new_slack(utils::lin &l) noexcept
    {
        auto var = vals.size();
        is_int.push_back(false);

        utils::inf_rational val(l.known_term), lb(l.known_term), ub(l.known_term);
        std::vector<utils::lit> lb_reason, ub_reason;
        for (const auto &[v, c] : l.vars)
        {
            val += c * vals[v];
            lb += (is_positive(c) ? c_bounds[lb_index(v)].value : c_bounds[ub_index(v)].value) * c;
            lb_reason.insert(lb_reason.end(), c_bounds[lb_index(v)].reason.cbegin(), c_bounds[lb_index(v)].reason.cend());
            ub += (is_positive(c) ? c_bounds[ub_index(v)].value : c_bounds[lb_index(v)].value) * c;
            ub_reason.insert(ub_reason.end(), c_bounds[ub_index(v)].reason.cbegin(), c_bounds[ub_index(v)].reason.cend());
        }

        return var;
    }

    void la_theory::propagate(const utils::lit &p) noexcept {}
} // namespace semitone