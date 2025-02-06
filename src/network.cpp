#include "network.hpp"
#include "la_theory.hpp"
#include <algorithm>
#include <cassert>

namespace semitone
{
    network::network() noexcept : la(new_theory<la_theory>(*this))
    {
        [[maybe_unused]] utils::var c_false = new_var(); // the false constant..
        assert(c_false == utils::FALSE_var);
        assigns[utils::FALSE_var] = utils::False;
        level[utils::FALSE_var] = 0;
    }

    utils::var network::new_var() noexcept
    {
        const auto x = assigns.size();
        assigns.push_back(utils::Undefined);
        watches.emplace_back();
        watches.emplace_back();
        level.emplace_back(0);
        reason.emplace_back(std::nullopt);
        return x;
    }

    utils::var network::new_int(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept { return la.new_int(lb, ub); }
    utils::var network::new_real(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept { return la.new_real(lb, ub); }

    bool network::add_clause(std::vector<utils::lit> &&lits) noexcept
    {
        assert(decision_level() == 0);
        // we check if the clause is already satisfied and filter out false/duplicate literals..
        std::sort(lits.begin(), lits.end()); // we sort the literals to easily remove duplicates..
        utils::lit p;
        size_t j = 0;
        for (auto it = lits.cbegin(); it != lits.cend(); ++it)
        {
            if (value(*it) == utils::True || *it == !p)
                return true; // the clause is already satisfied or is a tautology..
            if (value(*it) != utils::False && *it != p)
            { // we include this literal in the clause..
                p = *it;
                lits[j++] = p;
            }
        }
        lits.resize(j);

        switch (lits.size())
        {
        case 0:
            return false; // the clause is unsatisfiable..
        case 1:
            return enqueue(lits[0]); // the clause is unit under the current assignment..
        default:
            clauses.push_back(new clause(*this, std::move(lits))); // we add the clause to the problem..
            return true;
        }
    }

    void network::add_lt(utils::lin &lhs, utils::lin &rhs) { la.add_lt(lhs, rhs, true); }
    void network::add_le(utils::lin &lhs, utils::lin &rhs) noexcept { la.add_lt(lhs, rhs); }

    void network::new_lt(utils::lit &p, utils::lin &lhs, utils::lin &rhs) noexcept { la.new_lt(p, lhs, rhs, true); }
    void network::new_le(utils::lit &p, utils::lin &lhs, utils::lin &rhs) noexcept { la.new_lt(p, lhs, rhs); }

    bool network::enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c) noexcept
    {
        if (auto val = value(p); val != utils::Undefined)
            return val;
        assigns[variable(p)] = sign(p);
        level[variable(p)] = decision_level();
        if (c)
            reason[variable(p)] = c;
        trail.push_back(p);
        prop_queue.push(p);
        return true;
    }

    clause::clause(network &net, std::vector<utils::lit> &&lits) noexcept : net(net), lits(std::move(lits))
    {
        assert(lits.size() >= 2);
        net.watches[index(!lits[0])].emplace_back(*this);
        net.watches[index(!lits[1])].emplace_back(*this);
    }

    bool clause::propagate(const utils::lit &p) noexcept
    {
        assert(net.value(p) == utils::True);
        // make sure false literal is lits[1]..
        if (variable(lits[0]) == variable(p))
            std::swap(lits[0], lits[1]);
        assert(variable(lits[1]) == variable(p));

        // if 0th watch is true, the clause is already satisfied..
        if (net.value(lits[0]) == utils::True)
        {
            net.watches[index(p)].emplace_back(*this);
            return true;
        }

        // we look for a new literal to watch..
        for (size_t i = 1; i < lits.size(); ++i)
            if (net.value(lits[i]) != utils::False)
            {
                std::swap(lits[1], lits[i]);
                net.watches[index(!lits[1])].emplace_back(*this);
                return true;
            }

        // clause is unit under assignment..
        net.watches[index(p)].emplace_back(*this);
        return net.enqueue(lits[0]);
    }

    bool clause::simplify() noexcept
    {
        size_t j = 0;
        for (size_t i = 0; i < lits.size(); ++i)
            switch (net.value(lits[i]))
            {
            case utils::True:
                return true; // the clause is already satisfied..
            case utils::Undefined:
                lits[j++] = lits[i];
                break;
            }
        // the clause cannot be simplified..
        lits.resize(j);
        return false;
    }

    std::vector<utils::lit> clause::get_reason(const utils::lit &p) const noexcept
    {
        assert(is_undefined(p) || p == lits[0]);
        std::vector<utils::lit> r;
        r.reserve(is_undefined(p) ? lits.size() : lits.size() - 1);
        for (size_t i = is_undefined(p) ? 0 : 1; i < lits.size(); ++i)
        {
            assert(net.value(lits[i]) == utils::False); // when this function is called, the clause is unit..
            r.push_back(!lits[i]);
        }
        return r;
    }
} // namespace semitone