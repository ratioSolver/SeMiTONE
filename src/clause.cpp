#include <cassert>
#include <functional>
#include <algorithm>
#include "clause.hpp"

namespace semitone
{
    clause::clause(sat_core &s, std::vector<lit> &&ls) : constr(s), lits(std::move(ls))
    {
        assert(lits.size() >= 2);
        watches(!lits[0]).emplace_back(*this);
        watches(!lits[1]).emplace_back(*this);
    }

    std::unique_ptr<constr> clause::copy(sat_core &s) noexcept
    {
        std::vector<lit> lits;
        lits.reserve(this->lits.size());
        lits.insert(lits.begin(), this->lits.begin(), this->lits.end());
        return std::make_unique<clause>(s, std::move(lits));
    }

    bool clause::propagate(const lit &p) noexcept
    {
        // make sure false literal is lits[1]..
        if (variable(lits[0]) == variable(p))
            std::swap(*(lits.begin()), *(std::next(lits.begin())));
        assert(variable(lits[1]) == variable(p));

        // if 0th watch is true, the clause is already satisfied..
        if (value(lits[0]) == utils::True)
        {
            watches(p).emplace_back(*this);
            return true;
        }

        // we look for a new literal to watch..
        for (size_t i = 1; i < lits.size(); ++i)
            if (value(lits[i]) != utils::False)
            {
                std::swap(*(std::next(lits.begin())), *(std::next(lits.begin(), i)));
                watches(!lits[1]).emplace_back(*this);
                return true;
            }

        // clause is unit under assignment..
        watches(p).emplace_back(*this);
        return enqueue(lits[0]);
    }

    bool clause::simplify() noexcept
    {
        size_t j = 0;
        for (size_t i = 0; i < lits.size(); ++i)
            switch (value(lits[i]))
            {
            case utils::True:
                return true;
            case utils::Undefined:
                lits[j++] = lits[i];
                break;
            }
        lits.resize(j);
        return false;
    }

    std::vector<lit> clause::get_reason(const lit &p) const noexcept
    {
        assert(is_undefined(p) || p == lits[0]);
        std::vector<lit> r;
        r.reserve(is_undefined(p) ? lits.size() : lits.size() - 1);
        for (size_t i = is_undefined(p) ? 0 : 1; i < lits.size(); ++i)
        {
            assert(value(lits[i]) == utils::False);
            r.push_back(!lits[i]);
        }
        return r;
    }

    json::json clause::to_json() const noexcept
    {
        json::json j_lits(json::json_type::array);
        for (const auto &l : lits)
        {
            json::json j_lit{{"lit", to_string(l)}};
            switch (value(l))
            {
            case utils::True:
                j_lit["value"] = "true";
                break;
            case utils::False:
                j_lit["value"] = "false";
                break;
            }
            j_lits.push_back(std::move(j_lit));
        }
        return j_lits;
    }
} // namespace semitone
