#include "clause.h"
#include "sat_core.h"
#include <algorithm>
#include <cassert>

namespace semitone
{
    clause::clause(sat_core &s, std::vector<lit> lits) : constr(s), lits(std::move(lits)) {}

    clause *clause::new_clause(sat_core &s, std::vector<lit> lits)
    {
        auto l0 = lits[0], l1 = lits[1];
        clause *c = new clause(s, std::move(lits));
        c->watches(!l0).push_back(c);
        c->watches(!l1).push_back(c);
        return c;
    }

    bool clause::propagate(const lit &p) noexcept
    {
        // make sure false literal is lits[1]..
        if (variable(lits[0]) == variable(p))
            std::swap(*(lits.begin()), *(std::next(lits.begin())));

        // if 0th watch is true, the clause is already satisfied..
        if (value(lits[0]) == True)
        {
            watches(p).push_back(this);
            return true;
        }

        // we look for a new literal to watch..
        for (size_t i = 1; i < lits.size(); ++i)
            if (value(lits[i]) != False)
            {
                std::swap(*(std::next(lits.begin())), *(std::next(lits.begin(), i)));
                watches(!lits[1]).push_back(this);
                return true;
            }

        // clause is unit under assignment..
        watches(p).push_back(this);
        return enqueue(lits[0]);
    }

    bool clause::simplify() noexcept
    {
        size_t j = 0;
        for (size_t i = 0; i < lits.size(); ++i)
            switch (value(lits[i]))
            {
            case True:
                return true;
            case Undefined:
                lits[j++] = lits[i];
                break;
            }
        lits.resize(j);
        return false;
    }

    void clause::remove() noexcept
    {
        auto &l0_w = watches(!lits[0]);
        l0_w.erase(std::find(l0_w.cbegin(), l0_w.cend(), this));
        auto &l1_w = watches(!lits[1]);
        l1_w.erase(std::find(l1_w.cbegin(), l1_w.cend(), this));
        for (const auto &l : lits)
            remove_constr_from_reason(variable(l));
        delete this;
    }

    void clause::get_reason(const lit &p, std::vector<lit> &out_reason) const noexcept
    {
        assert(is_undefined(p) || p == lits[0]);
        out_reason.reserve(is_undefined(p) ? lits.size() : lits.size() - 1);
        for (size_t i = is_undefined(p) ? 0 : 1; i < lits.size(); ++i)
        {
            assert(value(lits[i]) == False);
            out_reason.push_back(!lits[i]);
        }
    }

    nlohmann::json clause::to_json() const noexcept
    {
        nlohmann::json j_cl;

        nlohmann::json::array_t j_lits;
        for (const auto &l : lits)
        {
            nlohmann::json j_lit;
            j_lit["lit"] = to_string(l);
            switch (value(l))
            {
            case True:
                j_lit["val"] = true;
                break;
            case False:
                j_lit["val"] = false;
                break;
            }
            j_lits.push_back(j_lit);
        }
        j_cl["lits"] = j_lits;

        return j_cl;
    }
} // namespace semitone