#include <unordered_map>
#include <set>
#include <algorithm>
#include <cassert>
#include "sat_core.hpp"
#include "clause.hpp"
#include "logging.hpp"

namespace semitone
{
    sat_core::sat_core()
    {
        [[maybe_unused]] VARIABLE_TYPE c_false = new_var(); // the false constant..
        assert(c_false == FALSE_var);
        assigns[FALSE_var] = utils::False;
        level[FALSE_var] = 0;
    }
    sat_core::sat_core(const sat_core &orig) : assigns(orig.assigns), level(orig.level), prop_queue(orig.prop_queue), trail(orig.trail), trail_lim(orig.trail_lim), decisions(orig.decisions)
    {
        assert(orig.prop_queue.empty());
        constrs.reserve(orig.constrs.size());
        watches.resize(orig.watches.size());
        std::unordered_map<constr *, constr *> old2new;
        for (const auto &c : orig.constrs)
        {
            constrs.push_back(c->copy(*this));
            old2new[c.get()] = constrs.back().get();
        }

        reason.reserve(orig.reason.size());
        for (const auto &r : orig.reason)
            if (r)
                reason.push_back(*old2new.at(&r->get()));
            else
                reason.push_back(std::nullopt);
    }

    VARIABLE_TYPE sat_core::new_var() noexcept
    {
        const VARIABLE_TYPE x = assigns.size();
        assigns.push_back(utils::Undefined);
        watches.emplace_back();
        watches.emplace_back();
        // exprs.emplace("b" + std::to_string(id), id);
        level.emplace_back(0);
        reason.emplace_back(std::nullopt);
        return x;
    }

    bool sat_core::new_clause(std::vector<lit> &&lits) noexcept
    {
        assert(root_level());
        // we check if the clause is already satisfied and filter out false/duplicate literals..
        std::sort(lits.begin(), lits.end());
        lit p;
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
            constrs.push_back(std::make_unique<clause>(*this, std::move(lits)));
            return true;
        }
    }

    bool sat_core::simplify_db() noexcept
    {
        assert(root_level());
        // if (!propagate())
        //     return false;
        size_t i = 0, j = constrs.size();
        while (i < j)
        {
            if (constrs[i]->simplify())
                constrs[i].swap(constrs[--j]);
            else
                ++i;
        }
        constrs.resize(j);
        return true;
    }

    bool sat_core::propagate() noexcept
    {
        lit p;
    main_loop:
        while (!prop_queue.empty())
        { // we first propagate sat constraints..
            p = prop_queue.front();
            prop_queue.pop();
            std::vector<std::reference_wrapper<semitone::constr>> ws;
            std::swap(watches[index(p)], ws);
            for (size_t i = 0; i < ws.size(); ++i)
                if (!ws[i].get().propagate(p))
                { // the constraint is conflicting..
                    for (size_t j = i + 1; j < ws.size(); ++j)
                        watches[index(p)].push_back(ws[j]); // we re-add the remaining watches..
                    while (!prop_queue.empty())
                        prop_queue.pop(); // we clear the propagation queue..

                    if (root_level())
                        return false; // the problem is unsatisfiable..

                    std::vector<lit> no_good;
                    size_t bt_level;
                    // we analyze the conflict..
                    analyze(ws[i].get(), no_good, bt_level);
                    while (decision_level() > bt_level)
                        pop();
                    // we record the no-good..
                    record(no_good);

                    goto main_loop;
                }
        }
        return true;
    }

    void sat_core::pop() noexcept
    {
        LOG_DEBUG("-[" << to_string(decisions.back()) << "]");
        while (trail_lim.back() < trail.size())
            pop_one();
        trail_lim.pop_back();
        decisions.pop_back();

        for (const auto &th : theories)
            th->pop();
    }

    bool sat_core::enqueue(const lit &p, const std::optional<std::reference_wrapper<constr>> &c) noexcept
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

    void sat_core::pop_one() noexcept
    {
        auto v = variable(trail.back());
        assigns[v] = utils::Undefined;
        level[v] = 0;
        reason[v].reset();
        trail.pop_back();
    }

    void sat_core::analyze(constr &cnfl, std::vector<lit> &out_learnt, size_t &out_btlevel) noexcept
    {
        std::set<VARIABLE_TYPE> seen;
        int counter = 0; // this is the number of variables of the current decision level that have already been seen..
        lit p;
        std::vector<lit> p_reason = cnfl.get_reason(p);
        out_learnt.push_back(p); // we make room for the next to be enqueued literal..
        out_btlevel = 0;
        do
        {
            // trace reason for `p`..
            for (const auto &q : p_reason) // the order in which these literals are visited is not relevant..
                if (seen.insert(variable(q)).second)
                {
                    assert(value(q) == utils::True); // this literal should have propagated the clause..
                    if (level[variable(q)] == decision_level())
                        counter++;
                    else if (level[variable(q)] > 0) // exclude variables from decision level 0..
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
                    p_reason = reason[variable(p)]->get().get_reason(p);
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

    void sat_core::record(std::vector<lit> lits) noexcept
    {
        assert(value(lits[0]) == utils::Undefined); // the asserting literal must be unassigned..
        assert(std::all_of(std::next(lits.cbegin()), lits.cend(), [this](auto &p)
                           { return value(p) == utils::False; })); // all these literals must have been assigned as false for propagating `lits[0]..
        if (lits.size() == 1)
        {
            assert(root_level());
            [[maybe_unused]] bool e = enqueue(lits[0]);
            assert(e);
        }
        else
        {
            // we sort literals according to descending order of variable assignment (except for the first literal which is now unassigned)..
            std::sort(std::next(lits.begin()), lits.end(), [this](auto &a, auto &b)
                      { return level[variable(a)] > level[variable(b)]; });

            auto l0 = lits[0];
            auto c = std::make_unique<clause>(*this, std::move(lits));
            [[maybe_unused]] bool e = enqueue(l0, *c);
            assert(e);
            constrs.push_back(std::move(c));
        }
    }
} // namespace semitone
