#include "ov_theory.h"
#include "ov_value_listener.h"
#include <algorithm>
#include <cassert>

namespace semitone
{
    SEMITONE_EXPORT ov_theory::ov_theory(sat_ptr sat) : theory(std::move(sat)) {}

    SEMITONE_EXPORT var ov_theory::new_var(const std::vector<utils::enum_val *> &items, const bool enforce_exct_one) noexcept
    {
        assert(!items.empty());
        const var id = assigns.size();
        auto c_vals = std::unordered_map<utils::enum_val *, lit>();
        if (items.size() == 1)
            c_vals.emplace(*items.cbegin(), TRUE_lit);
        else
        {
            for (const auto &i : items)
            {
                const var bv = sat->new_var();
                c_vals.emplace(i, lit(bv));
                bind(bv);
                is_contained_in[bv].insert(id);
            }
            if (enforce_exct_one)
            {
                std::vector<lit> lits;
                lits.reserve(items.size());
                for (const auto &i : items)
                    lits.push_back(c_vals.find(i)->second);
                [[maybe_unused]] bool exct_one = sat->new_clause({sat->new_exct_one(std::move(lits))});
                assert(exct_one);
            }
        }
        assigns.push_back(c_vals);
        return id;
    }

    SEMITONE_EXPORT var ov_theory::new_var(const std::vector<lit> &lits, const std::vector<utils::enum_val *> &vals) noexcept
    {
        assert(!lits.empty());
        assert(lits.size() == vals.size());
        const var id = assigns.size();
        auto c_vals = std::unordered_map<utils::enum_val *, lit>();
        for (size_t i = 0; i < lits.size(); ++i)
        {
            c_vals.emplace(vals[i], lits[i]);
            is_contained_in[variable(lits[i])].insert(id);
        }
        assigns.push_back(c_vals);
        return id;
    }

    SEMITONE_EXPORT lit ov_theory::allows(const var &v, utils::enum_val &val) const noexcept
    {
        if (auto at_right = assigns[v].find(&val); at_right != assigns[v].cend())
            return at_right->second;
        else
            return FALSE_lit;
    }

    SEMITONE_EXPORT lit ov_theory::new_eq(const var &left, const var &right) noexcept
    {
        if (left == right)
            return TRUE_lit;

        if (left > right)
            return new_eq(right, left);

        const std::string s_expr = left < right ? ("=e" + std::to_string(left) + "e" + std::to_string(right)) : ("=e" + std::to_string(right) + "e" + std::to_string(left));
        if (const auto at_expr = exprs.find(s_expr); at_expr != exprs.cend()) // the expression already exists..
            return at_expr->second;
        else
        {
            std::unordered_set<utils::enum_val *> intersection;
            for ([[maybe_unused]] const auto &[val, l] : assigns[left])
                if (assigns[right].count(val))
                    intersection.insert(val);

            if (intersection.empty())
                return FALSE_lit;

            // we need to create a new variable..
            const lit eq_lit = lit(sat->new_var()); // the equality literal..

            [[maybe_unused]] bool nc;
            // the values outside the intersection are pruned if the equality control variable becomes true..
            for (const auto &[val, l] : assigns[left])
                if (!intersection.count(val))
                {
                    nc = sat->new_clause({!eq_lit, !l});
                    assert(nc);
                }
            for (const auto &[val, l] : assigns[right])
                if (!intersection.count(val))
                {
                    nc = sat->new_clause({!eq_lit, !l});
                    assert(nc);
                }
            // the values inside the intersection are made pairwise equal if the equality variable becomes true..
            for (const auto &v : intersection)
            {
                nc = sat->new_clause({!eq_lit, !assigns[left].at(v), assigns[right].at(v)});
                assert(nc);
                nc = sat->new_clause({!eq_lit, assigns[left].at(v), !assigns[right].at(v)});
                assert(nc);
                nc = sat->new_clause({eq_lit, !assigns[left].at(v), !assigns[right].at(v)});
                assert(nc);
            }

            exprs.emplace(s_expr, eq_lit);
            return eq_lit;
        }
    }

    SEMITONE_EXPORT std::unordered_set<utils::enum_val *> ov_theory::value(var v) const noexcept
    {
        std::unordered_set<utils::enum_val *> vals;
        for (const auto &[val, l] : assigns[v])
            if (sat->value(l) != utils::False)
                vals.insert(val);
        return vals;
    }

    SEMITONE_EXPORT void ov_theory::listen(const var &v, ov_value_listener *const l) noexcept
    {
        auto domain = value(v);
        if (domain.size() > 1)
        {
            for (const auto &val : domain)
                l->listen_sat(variable(assigns[v].at(val)));
        }
    }
} // namespace semitone