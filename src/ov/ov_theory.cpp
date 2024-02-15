#include <cassert>
#include "ov_theory.hpp"
#include "sat_core.hpp"
#include "ov_eq.hpp"

namespace semitone
{
    ov_theory::ov_theory(std::shared_ptr<sat_core> sat) : theory(sat) {}

    VARIABLE_TYPE ov_theory::new_var(std::vector<std::reference_wrapper<utils::enum_val>> &&domain, const bool enforce_exct_one) noexcept
    {
        assert(!domain.empty());
        const auto x = domains.size();
        domains.emplace_back();
        exact_one.push_back(enforce_exct_one);
        if (domain.size() == 1 && enforce_exct_one)
            domains[x].emplace_back(domain[0].get(), TRUE_lit);
        else
            for (const auto &v : domain)
            {
                const auto bv = sat->new_var();
                domains[x].emplace_back(v.get(), bv);
                bind(bv);
            }
        return x;
    }

    VARIABLE_TYPE ov_theory::new_var(std::vector<std::pair<std::reference_wrapper<utils::enum_val>, lit>> &&domain) noexcept
    {
        assert(!domain.empty());
        const auto x = domains.size();
        domains.emplace_back();
        exact_one.push_back(false);
        for (const auto &v : domain)
        {
            domains[x].emplace_back(v.first.get(), v.second);
            bind(variable(v.second));
        }
        return x;
    }

    lit ov_theory::new_eq(const VARIABLE_TYPE left, const VARIABLE_TYPE right) noexcept
    {
        if (left == right)
            return TRUE_lit;
        const auto ctr = lit(sat->new_var());
        sat->add_constr(std::make_unique<ov_eq>(*this, left, right, ctr));
        return ctr;
    }

    std::vector<std::reference_wrapper<utils::enum_val>> ov_theory::domain(const VARIABLE_TYPE var) noexcept
    {
        std::vector<std::reference_wrapper<utils::enum_val>> d;
        for (const auto &v : domains[var])
            if (sat->value(v.second) != utils::False)
                d.push_back(v.first.get());
        return d;
    }
} // namespace semitone
