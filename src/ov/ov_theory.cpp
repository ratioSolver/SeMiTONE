#include <unordered_set>
#include <cassert>
#include "ov_theory.hpp"
#include "sat_core.hpp"

namespace semitone
{
    VARIABLE_TYPE ov_theory::new_var(std::vector<std::reference_wrapper<utils::enum_val>> &&domain, const bool enforce_exct_one) noexcept
    {
        assert(!domain.empty());
        const auto x = domains.size();
        domains.emplace_back();
        if (domain.size() == 1 && enforce_exct_one)
            domains[x].emplace(&domain[0].get(), utils::TRUE_lit);
        else
        {
            for (const auto &v : domain)
            {
                const auto bv = sat->new_var();
                domains[x].emplace(&v.get(), bv);
            }
            if (enforce_exct_one)
            {
                for (size_t i = 0; i < domain.size(); ++i)
                    for (size_t j = i + 1; j < domain.size(); ++j)
                        if (!sat->new_clause({!domains[x].at(&domain[i].get()), !domains[x].at(&domain[j].get())}))
                            return -1;

                std::vector<utils::lit> lits;
                lits.reserve(domain.size());
                for (const auto &v : domain)
                    lits.push_back(domains[x].at(&v.get()));
                if (!sat->new_clause(std::move(lits)))
                    return -1;
            }
        }
        return x;
    }

    VARIABLE_TYPE ov_theory::new_var(std::vector<std::pair<std::reference_wrapper<utils::enum_val>, utils::lit>> &&domain) noexcept
    {
        assert(!domain.empty());
        const auto x = domains.size();
        domains.emplace_back();
        for (const auto &v : domain)
            domains[x].emplace(&v.first.get(), v.second);
        return x;
    }

    utils::lit ov_theory::new_eq(const VARIABLE_TYPE left, const VARIABLE_TYPE right) noexcept
    {
        if (left == right)
            return utils::TRUE_lit;

        // we compute the intersection of the two domains
        std::unordered_set<utils::enum_val *> intersection;
        for (const auto &v : domains[left])
            if (domains[right].count(v.first))
                intersection.insert(v.first);
        if (intersection.empty())
            return utils::FALSE_lit; // the domains are disjoint

        // we need to create a new variable..
        const auto ctr = utils::lit(sat->new_var());

        // the values outside the intersection are pruned if the equality control variable becomes true..
        for (const auto &[val, l] : domains[left])
            if (!intersection.count(val))
                if (!sat->new_clause({!ctr, !l}))
                    return utils::FALSE_lit;
        for (const auto &[val, l] : domains[right])
            if (!intersection.count(val))
                if (!sat->new_clause({!ctr, !l}))
                    return utils::FALSE_lit;

        // the values inside the intersection are made equal if the equality control variable becomes true..
        for (const auto val : intersection)
            if (!sat->new_clause({!ctr, domains[left].at(val), !domains[right].at(val)}) || !sat->new_clause({!ctr, !domains[left].at(val), domains[right].at(val)}) || !sat->new_clause({ctr, !domains[left].at(val), !domains[right].at(val)}))
                return utils::FALSE_lit;

        return ctr;
    }

    std::vector<std::reference_wrapper<utils::enum_val>> ov_theory::domain(const VARIABLE_TYPE var) const noexcept
    {
        std::vector<std::reference_wrapper<utils::enum_val>> d;
        for (const auto &v : domains[var])
            if (sat->value(v.second) != utils::False)
                d.push_back(*v.first);
        return d;
    }

    bool ov_theory::assign(const VARIABLE_TYPE var, utils::enum_val &val) noexcept { return sat->assume(domains[var].at(&val)); }
    bool ov_theory::forbid(const VARIABLE_TYPE var, utils::enum_val &val) noexcept { return sat->assume(!domains[var].at(&val)); }
} // namespace semitone
