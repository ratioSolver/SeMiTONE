#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cassert>
#include "sat_core.hpp"
#include "clause.hpp"
#include "logging.hpp"

#ifdef BUILD_LISTENERS
#include "sat_value_listener.hpp"
#define FIRE_ON_VALUE_CHANGED(var)                                       \
    if (const auto &at_v = listening.find(var); at_v != listening.end()) \
    {                                                                    \
        for (auto &l : at_v->second)                                     \
            l->on_sat_value_changed(var);                                \
        if (root_level())                                                \
            listening.erase(at_v);                                       \
    }
#define FIRE_ON_VALUE_RESET(var)                                         \
    if (const auto &at_v = listening.find(var); at_v != listening.end()) \
    {                                                                    \
        for (auto &l : at_v->second)                                     \
            l->on_sat_value_changed(var);                                \
    }
#else
#define FIRE_ON_VALUE_CHANGED(var)
#define FIRE_ON_VALUE_RESET(var)
#endif

namespace semitone
{
    sat_core::sat_core() noexcept
    {
        [[maybe_unused]] VARIABLE_TYPE c_false = new_var(); // the false constant..
        assert(c_false == utils::FALSE_var);
        assigns[utils::FALSE_var] = utils::False;
        level[utils::FALSE_var] = 0;
    }
#ifdef BUILD_LISTENERS
    sat_core::sat_core(const sat_core &orig) noexcept : assigns(orig.assigns), level(orig.level), prop_queue(orig.prop_queue), trail(orig.trail), trail_lim(orig.trail_lim), decisions(orig.decisions), listening(orig.listening), listeners(orig.listeners)
#else
    sat_core::sat_core(const sat_core &orig) noexcept : assigns(orig.assigns), level(orig.level), prop_queue(orig.prop_queue), trail(orig.trail), trail_lim(orig.trail_lim), decisions(orig.decisions)
#endif
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

#ifdef BUILD_LISTENERS
        for (auto l : orig.listeners)
            l->sat = this;
#endif
    }

    VARIABLE_TYPE sat_core::new_var() noexcept
    {
        const auto x = assigns.size();
        assigns.push_back(utils::Undefined);
        watches.emplace_back();
        watches.emplace_back();
        // exprs.emplace("b" + std::to_string(id), id);
        level.emplace_back(0);
        reason.emplace_back(std::nullopt);
        return x;
    }

    bool sat_core::new_clause(std::vector<utils::lit> &&lits) noexcept
    {
        assert(root_level());
        // we check if the clause is already satisfied and filter out false/duplicate literals..
        std::sort(lits.begin(), lits.end());
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
            constrs.push_back(std::make_unique<clause>(*this, std::move(lits)));
            LOG_DEBUG("+" << constrs.back()->to_json());
            return true;
        }
    }

    utils::lit sat_core::new_eq(const utils::lit &left, const utils::lit &right) noexcept
    {
        assert(root_level());
        // we try to avoid creating a new variable..
        if (left == right)
            return utils::TRUE_lit; // the variables are the same variable..
        switch (value(left))
        {
        case utils::True:
            return right; // the equality is already satisfied..
        case utils::False:
            return !right; // the equality is already unsatisfied..
        default:
            switch (value(right))
            {
            case utils::True:
                return left; // the equality is already satisfied..
            case utils::False:
                return !left; // the equality is already unsatisfied..
            default:
                const auto ctr = utils::lit(new_var());
                if (!new_clause({ctr, left, right}) || !new_clause({ctr, !left, !right}) || !new_clause({!ctr, !left, right}) || !new_clause({!ctr, left, !right}))
                    return utils::FALSE_lit; // the equality is unsatisfiable..
                return ctr;
            }
        }
    }

    utils::lit sat_core::new_conj(std::vector<utils::lit> &&ls) noexcept
    {
        assert(root_level());
        // we try to avoid creating a new variable..
        std::sort(ls.begin(), ls.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        utils::lit p;
        size_t j = 0;
        for (auto it = ls.cbegin(); it != ls.cend(); ++it)
            if (value(*it) == utils::False || *it == !p)
                return utils::FALSE_lit; // the conjunction cannot be satisfied..
            else if (value(*it) != utils::True && *it != p)
            { // we need to include this literal in the conjunction..
                p = *it;
                ls[j++] = p;
            }
        ls.resize(j);

        if (ls.empty()) // an empty conjunction is assumed to be satisfied..
            return utils::TRUE_lit;
        else if (ls.size() == 1)
            return ls[0]; // the conjunction is unit under the current assignment..
        else
        { // we need to create a new variable..
            const auto ctr = utils::lit(new_var());
            std::vector<utils::lit> lits;
            lits.reserve(ls.size() + 1);
            lits.push_back(ctr);
            for (const auto &l : ls)
            {
                if (!new_clause({!ctr, l}))
                    return utils::FALSE_lit; // the conjunction is unsatisfiable..
                lits.push_back(!l);
            }
            if (!new_clause(std::move(lits)))
                return utils::FALSE_lit; // the conjunction is unsatisfiable..
            return ctr;
        }
    }

    utils::lit sat_core::new_disj(std::vector<utils::lit> &&ls) noexcept
    {
        assert(root_level());
        // we try to avoid creating a new variable..
        std::sort(ls.begin(), ls.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        utils::lit p;
        size_t j = 0;
        for (auto it = ls.cbegin(); it != ls.cend(); ++it)
            if (value(*it) == utils::True || *it == !p)
                return utils::TRUE_lit; // the disjunction is already satisfied..
            else if (value(*it) != utils::False && *it != p)
            { // we need to include this literal in the conjunction..
                p = *it;
                ls[j++] = p;
            }
        ls.resize(j);

        if (ls.empty()) // an empty disjunction is assumed to be unsatisfable..
            return utils::FALSE_lit;
        else if (ls.size() == 1)
            return ls[0]; // the disjunction is unit under the current assignment..
        else
        { // we need to create a new variable..
            const auto ctr = utils::lit(new_var());
            std::vector<utils::lit> lits;
            lits.reserve(ls.size() + 1);
            lits.push_back(!ctr);
            for (const auto &l : ls)
            {
                if (!new_clause({ctr, !l}))
                    return utils::FALSE_lit; // the disjunction is unsatisfiable..
                lits.push_back(l);
            }
            if (!new_clause(std::move(lits)))
                return utils::FALSE_lit; // the disjunction is unsatisfiable..
            return ctr;
        }
    }

    utils::lit sat_core::new_at_most_one(std::vector<utils::lit> &&ls) noexcept
    {
        assert(root_level());
        // we try to avoid creating a new variable..
        std::sort(ls.begin(), ls.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        bool true_found = false;
        utils::lit p;
        size_t j = 0;
        for (auto it = ls.cbegin(); it != ls.cend(); ++it)
            switch (value(*it))
            {
            case utils::True:
                if (true_found)
                    return utils::FALSE_lit; // more than one literal is `true`, the at-most-one is `false`
                true_found = true;
                [[fallthrough]];
            case utils::Undefined:
                p = *it;
                ls[j++] = p;
                break;
            }
        ls.resize(j);

        if (ls.empty())
            return utils::TRUE_lit; // the at-most-one is satisfied..
        else if (ls.size() == 1)
            return ls[0]; // the at-most-one is unit under the current assignment..
        else
        { // we need to create a new variable..
            const auto ctr = utils::lit(new_var());
            for (size_t i = 0; i < ls.size(); ++i)
                for (size_t j = i + 1; j < ls.size(); ++j)
                    if (!new_clause({!ctr, !ls[i], !ls[j]})) // if two literals are `true`, the at-most-one is `false`..
                        return utils::FALSE_lit;             // the at-most-one is unsatisfiable..
            std::vector<utils::lit> lits = ls;
            lits.push_back(ctr);
            if (!new_clause(std::move(lits))) // if no literal is `true`, the at-most-one is `true`..
                return utils::FALSE_lit;      // the at-most-one is unsatisfiable..
            for (size_t i = 0; i < ls.size(); ++i)
            {
                std::vector<utils::lit> c_lits = ls;
                c_lits[i] = !c_lits[i];
                c_lits.push_back(ctr);
                if (!new_clause(std::move(c_lits))) // if all literals except one are `false`, the at-most-one is `true`..
                    return utils::FALSE_lit;        // the at-most-one is unsatisfiable..
            }
            return ctr;
        }
    }

    utils::lit sat_core::new_exact_one(std::vector<utils::lit> &&ls) noexcept
    {
        assert(root_level());
        // we try to avoid creating a new variable..
        std::sort(ls.begin(), ls.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        bool true_found = false;
        utils::lit p;
        size_t j = 0;
        for (auto it = ls.cbegin(); it != ls.cend(); ++it)
            switch (value(*it))
            {
            case utils::True:
                if (true_found)
                    return utils::FALSE_lit; // more than one literal is `true`, the exact-one is `false`
                true_found = true;
                [[fallthrough]];
            case utils::Undefined:
                p = *it;
                ls[j++] = p;
                break;
            }
        ls.resize(j);

        if (ls.empty())
            return utils::FALSE_lit; // the exact-one is `false`
        else if (ls.size() == 1)
            return ls[0]; // the exact-one is unit under the current assignment..
        else
        { // we need to create a new variable..
            const auto ctr = utils::lit(new_var());
            for (size_t i = 0; i < ls.size(); ++i)
                for (size_t j = i + 1; j < ls.size(); ++j)
                    if (!new_clause({!ctr, !ls[i], !ls[j]})) // if two literals are `true`, the exact-one is `false`..
                        return utils::FALSE_lit;             // the exact-one is unsatisfiable..
            std::vector<utils::lit> lits = ls;
            lits.push_back(!ctr);
            if (!new_clause(std::move(lits))) // if no literal is `true`, the exact-one is `false`..
                return utils::FALSE_lit;      // the exact-one is unsatisfiable..
            for (size_t i = 0; i < ls.size(); ++i)
            {
                std::vector<utils::lit> c_lits = ls;
                c_lits[i] = !c_lits[i];
                c_lits.push_back(ctr);
                if (!new_clause(std::move(c_lits))) // if exactly one literal is `true`, the exact-one is `true`..
                    return utils::FALSE_lit;        // the exact-one is unsatisfiable..
            }
            return ctr;
        }
    }

    bool sat_core::assume(const utils::lit &p) noexcept
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

    bool sat_core::simplify_db() noexcept
    {
        assert(root_level());
        if (!propagate())
            return false;
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
        utils::lit p;
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

                    std::vector<utils::lit> no_good;
                    size_t bt_level;
                    // we analyze the conflict..
                    analyze(ws[i].get(), no_good, bt_level);
                    while (decision_level() > bt_level)
                        pop();
                    // we record the no-good..
                    record(no_good);

                    goto main_loop;
                }

            // we then perform theory propagation..
            if (const auto bnds_it = binds.find(variable(p)); bnds_it != binds.cend())
            {
                for (const auto &th : bnds_it->second)
                    if (!th.get().propagate(p))
                    { // the theory is conflicting..
                        while (!prop_queue.empty())
                            prop_queue.pop();

                        if (root_level())
                        { // the problem is unsatisfiable..
                            th.get().cnfl.clear();
                            return false;
                        }

                        // we analyze the theory's conflict, create a no-good from the analysis and backjump..
                        th.get().analyze_and_backjump();
                        goto main_loop;
                    }
                if (root_level()) // since this variable will no more be assigned, we can perform some cleanings..
                    binds.erase(bnds_it);
            }
        }

        // finally, we check theories..
        for (const auto &th : theories)
            if (!th->check())
            { // the theory is conflicting..
                if (root_level())
                { // the problem is unsatisfiable..
                    th->cnfl.clear();
                    return false;
                }

                // we analyze the theory's conflict, create a no-good from the analysis and backjump..
                th->analyze_and_backjump();
                goto main_loop;
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

    bool sat_core::enqueue(const utils::lit &p, const std::optional<std::reference_wrapper<constr>> &c) noexcept
    {
        if (auto val = value(p); val != utils::Undefined)
            return val;
        assigns[variable(p)] = sign(p);
        level[variable(p)] = decision_level();
        if (c)
            reason[variable(p)] = c;
        trail.push_back(p);
        prop_queue.push(p);
        FIRE_ON_VALUE_CHANGED(variable(p));
        return true;
    }

    void sat_core::pop_one() noexcept
    {
        auto v = variable(trail.back());
        assigns[v] = utils::Undefined;
        level[v] = 0;
        reason[v].reset();
        trail.pop_back();
        FIRE_ON_VALUE_RESET(v);
    }

    void sat_core::analyze(constr &cnfl, std::vector<utils::lit> &out_learnt, size_t &out_btlevel) noexcept
    {
        std::set<VARIABLE_TYPE> seen;
        int counter = 0; // this is the number of variables of the current decision level that have already been seen..
        utils::lit p;
        std::vector<utils::lit> p_reason = cnfl.get_reason(p);
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

    void sat_core::record(std::vector<utils::lit> lits) noexcept
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

#ifdef BUILD_LISTENERS
    void sat_core::add_listener(sat_value_listener &l) noexcept
    {
        l.sat = this;
        listeners.insert(&l);
    }
    void sat_core::remove_listener(sat_value_listener &l) noexcept
    {
        l.sat = nullptr;
        for (auto v : l.listening)
        {
            listening[v].erase(&l);
            if (listening[v].empty())
                listening.erase(v);
        }
        listeners.erase(&l);
    }
#endif
} // namespace semitone
