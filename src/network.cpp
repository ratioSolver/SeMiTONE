#include "network.hpp"
#include "la_theory.hpp"
#include "logging.hpp"
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

    void network::add_clause(std::vector<utils::lit> &&lits)
    {
        assert(decision_level() == 0);
        // we check if the clause is already satisfied and filter out false/duplicate literals..
        std::sort(lits.begin(), lits.end()); // we sort the literals to easily remove duplicates..
        utils::lit p;
        size_t j = 0;
        for (auto it = lits.cbegin(); it != lits.cend(); ++it)
        {
            if (value(*it) == utils::True || *it == !p)
                return; // the clause is already satisfied or is a tautology..
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
            throw unsolvable_exception(); // the problem is unsolvable..
        case 1:
            LOG_TRACE("( " << to_string(lits[0]) << " )");
            if (!enqueue(lits[0]))            // the clause is unit under the current assignment..
                throw unsolvable_exception(); // the problem is unsolvable..
            break;
        default:
            auto c = new clause(*this, std::move(lits));
            LOG_TRACE(*c);
            clauses.emplace_back(c); // we add the clause to the problem..
        }
    }

    void network::add_lt(utils::lin &lhs, utils::lin &rhs) { la.add_lt(lhs, rhs, true); }
    void network::add_le(utils::lin &lhs, utils::lin &rhs) noexcept { la.add_lt(lhs, rhs); }

    void network::new_lt(utils::lit &p, utils::lin &lhs, utils::lin &rhs) noexcept { la.new_lt(p, lhs, rhs, true); }
    void network::new_le(utils::lit &p, utils::lin &lhs, utils::lin &rhs) noexcept { la.new_lt(p, lhs, rhs); }

    bool network::assume(const utils::lit &p) noexcept
    {
        assert(value(p) == utils::Undefined);
        assert(prop_queue.empty());
        LOG_DEBUG("+[" << to_string(p) << "]");
        trail_lim.push_back(trail.size());
        decisions.push_back(p);
        for (const auto &th : theories)
            th->push();
        return enqueue(p) && propagate();
    }

    bool network::simplify_db() noexcept
    {
        assert(decision_level() == 0);
        if (!propagate())
            return false;
        size_t i = 0, j = clauses.size();
        while (i < j)
        {
            if (clauses[i]->simplify())
                clauses[i].swap(clauses[--j]);
            else
                ++i;
        }
        clauses.resize(j);
        return true;
    }

    bool network::propagate() noexcept
    {
        utils::lit p;
    main_loop:
        while (!prop_queue.empty())
        { // we first propagate sat constraints..
            p = prop_queue.front();
            prop_queue.pop();
            std::vector<utils::ref_wrapper<clause>> ws;
            std::swap(watches[index(p)], ws);
            for (size_t i = 0; i < ws.size(); ++i)
                if (!ws[i]->propagate(p))
                { // the constraint is conflicting..
                    for (size_t j = i + 1; j < ws.size(); ++j)
                        watches[index(p)].push_back(ws[j]); // we re-add the remaining watches..
                    while (!prop_queue.empty())
                        prop_queue.pop(); // we clear the propagation queue..

                    if (decision_level() == 0)
                        return false; // the problem is unsatisfiable..

                    std::vector<utils::lit> no_good;
                    size_t bt_level;
                    // we analyze the conflict..
                    analyze(*ws[i], no_good, bt_level);
                    while (decision_level() > bt_level)
                        pop();
                    // we record the no-good..
                    record(std::move(no_good));

                    goto main_loop;
                }

            // we then perform theory propagation..
            if (const auto bnds_it = binds.find(variable(p)); bnds_it != binds.cend())
            {
                for (const auto &th : bnds_it->second)
                    if (!th->propagate(p)) // the theory is conflicting..
                    {
                        while (!prop_queue.empty())
                            prop_queue.pop();

                        if (decision_level() == 0) // the problem is unsatisfiable..
                            return false;

                        // we analyze the theory's conflict, create a no-good from the analysis and backjump..
                        clause cnfl_cl(*this, std::move(th->cnfl));

                        // .. and we analyze the conflict..
                        std::vector<utils::lit> no_good;
                        size_t bt_level = 0;
                        analyze(cnfl_cl, no_good, bt_level);

                        // we backjump..
                        while (decision_level() > bt_level)
                            pop();
                        // .. and record the no-good..
                        record(std::move(no_good));
                        goto main_loop;
                    }
                if (decision_level() == 0) // since this variable will no more be assigned, we can perform some cleanings..
                    binds.erase(bnds_it);
            }
        }

        // finally, we check theories..
        for (const auto &th : theories)
            if (!th->check()) // the theory is conflicting..
            {
                assert(prop_queue.empty());

                if (decision_level() == 0) // the problem is unsatisfiable..
                    return false;

                assert(!th->cnfl.empty());
                if (th->cnfl.size() == 1)
                {
                    while (decision_level() > 0)
                        pop();
                    if (!enqueue(th->cnfl[0]))
                        return false;
                    goto main_loop;
                }

                // we analyze the theory's conflict, create a no-good from the analysis and backjump..
                clause cnfl_cl(*this, std::move(th->cnfl));

                // .. and we analyze the conflict..
                std::vector<utils::lit> no_good;
                size_t bt_level = 0;
                analyze(cnfl_cl, no_good, bt_level);

                // we backjump..
                while (decision_level() > bt_level)
                    pop();
                // .. and record the no-good..
                record(std::move(no_good));
                goto main_loop;
            }

        return true;
    }

    void network::pop() noexcept
    {
        LOG_DEBUG("-[" << to_string(decisions.back()) << "]");
        while (trail_lim.back() < trail.size())
            pop_one();
        trail_lim.pop_back();
        decisions.pop_back();

        for (const auto &th : theories)
            th->pop();
    }

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

    void network::pop_one() noexcept
    {
        auto v = variable(trail.back());
        assigns[v] = utils::Undefined;
        level[v] = 0;
        reason[v].reset();
        trail.pop_back();
    }

    void network::analyze(clause &cnfl, std::vector<utils::lit> &out_learnt, size_t &out_btlevel) noexcept
    {
        std::set<utils::var> seen;
        int counter = 0; // this is the number of variables of the current decision level that have already been seen..
        utils::lit p;
        std::vector<utils::lit> p_reason = cnfl.get_reason(p);
        out_learnt.push_back(p); // we make room for the next to be enqueued literal..
        out_btlevel = 0;
        do
        {
            // trace reason for `p`..
            for (const auto &q : p_reason) // the order in which these literals are visited is not relevant..
                if (level[variable(q)] > 0 && seen.insert(variable(q)).second)
                {
                    assert(value(q) == utils::True); // this literal should have propagated the clause..
                    if (level[variable(q)] >= decision_level())
                        counter++;
                    else
                    {
                        out_learnt.push_back(!q); // this literal has been assigned in a previous decision level..
                        out_btlevel = std::max(out_btlevel, level[variable(q)]);
                    }
                }
            do
            { // select next literal to look at..
                p = trail.back();
                assert(level[variable(p)] == decision_level()); // this variable must have been assigned at the current decision level..
                if (reason[variable(p)])                        // `p` can be the asserting literal..
                    p_reason = reason[variable(p)].value()->get_reason(p);
                pop_one();
            } while (!seen.count(variable(p)));
            counter--;
        } while (counter > 0);
        // `p` is now the first Unique Implication Point (UIP), possibly the asserting literal, that led to the conflict..
        assert(value(p) == utils::Undefined);
        assert(std::all_of(std::next(out_learnt.cbegin()), out_learnt.cend(), [this](auto &lt)
                           { return value(lt) == utils::False; })); // all these literals must have been assigned as false for propagating `p`..
        out_learnt[0] = !p;                                         // the asserting literal..
    }

    void network::record(std::vector<utils::lit> &&lits) noexcept
    {
        assert(value(lits[0]) == utils::Undefined); // the asserting literal must be unassigned..
        assert(std::all_of(std::next(lits.cbegin()), lits.cend(), [this](auto &p)
                           { return value(p) == utils::False; })); // all these literals must have been assigned as false for propagating `lits[0]..
        if (lits.size() == 1)
        {
            assert(decision_level() == 0);
            [[maybe_unused]] bool e = enqueue(lits[0]);
            assert(e);
        }
        else
        {
            // we sort literals according to descending order of variable assignment (except for the first literal which is now unassigned)..
            std::sort(std::next(lits.begin()), lits.end(), [this](auto &a, auto &b)
                      { return level[variable(a)] > level[variable(b)]; });

            auto l0 = lits[0];
            auto c = new clause(*this, std::move(lits));
            LOG_TRACE(*c);
            [[maybe_unused]] bool e = enqueue(l0, *c);
            assert(e);
            clauses.emplace_back(c);
        }
    }

    clause::clause(network &net, std::vector<utils::lit> &&ls) noexcept : net(net), lits(std::move(ls))
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
        return net.enqueue(lits[0], *this);
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

    [[nodiscard]] std::ostream &operator<<(std::ostream &os, const clause &c)
    {
        os << "(" << to_string(c.lits[0]);
        for (size_t i = 1; i < c.lits.size(); ++i)
            os << " ∨ " << to_string(c.lits[i]);
        return os << ")";
    }
} // namespace semitone