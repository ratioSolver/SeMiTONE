#include "network.hpp"
#include "logging.hpp"
#include <algorithm>
#include <set>
#include <cassert>

namespace semitone
{
    network::network(context &ctx) : ctx(ctx) {}

    void network::add(bool_expr expr)
    {
        auto cnf_expr = ctx.to_cnf(expr); // Convert to CNF
        if (auto and_xpr = utils::s_ptr_cast<and_expr>(cnf_expr))
            for (const auto &arg : and_xpr->args())
                add_term(arg);
        else
            add_term(cnf_expr);
    }

    bool network::propagate() noexcept
    {
        utils::lit p;
    main_loop:
        while (!prop_queue.empty())
        { // we first propagate clauses..
            p = prop_queue.front();
            prop_queue.pop();
            std::vector<utils::ref_wrapper<clause>> ws;
            std::swap(watches[index(p)], ws);
            for (size_t i = 0; i < ws.size(); ++i)
                if (!ws[i]->propagate(p))
                { // the clause is not propagating..
                    for (size_t j = i + 1; j < ws.size(); ++j)
                        watches[index(p)].push_back(ws[j]); // we re-add the remaining watches..
                    while (!prop_queue.empty())
                        prop_queue.pop(); // we clear the propagation queue..

                    if (decision_level() == 0)
                        return false; // the problem is unsatisfiable..

                    // we analyze the conflict..
                    std::vector<utils::lit> no_good;
                    size_t bt_level;
                    analyze(*ws[i], no_good, bt_level);
                    while (decision_level() > bt_level)
                        pop();
                    // we record the no-good..
                    record(no_good);

                    goto main_loop;
                }
        }

        return true;
    }

    bool network::assume(bool_expr expr) noexcept
    {
        utils::lit p;
        if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
            p = utils::lit(var_map.at(not_xpr->arg()->get_name()), false);
        else
            p = utils::lit(var_map.at(expr->get_name()), true);
        assert(value(p) == utils::Undefined);
        assert(prop_queue.empty());
        LOG_TRACE("+[" << to_string(p) << "]");
        trail_lim.push_back(trail.size());
        // TODO: Push the theories
        // for (const auto &th : theories)
        //     th->push();
        return enqueue(p) && propagate();
    }

    void network::pop() noexcept
    {
        LOG_TRACE("-[" << to_string(decisions.back()) << "]");
        while (trail_lim.back() < trail.size())
            pop_one();
        trail_lim.pop_back();
        // TODO: Pop the theories
        // for (const auto &th : theories)
        //     th->pop();
    }

    utils::lbool network::eval(bool_expr xpr)
    {
        if (auto bool_xpr = utils::s_ptr_cast<bool_var>(xpr))
        { // we evaluate the variable..
            if (auto it = var_map.find(bool_xpr->get_name()); it != var_map.end())
                return value(it->second);
            else
                return bool_xpr->val();
        }
        else if (auto not_xpr = utils::s_ptr_cast<not_expr>(xpr))
        { // we evaluate the expression recursively..
            eval(not_xpr->arg());
            return not_xpr->val();
        }
        else if (auto and_xpr = utils::s_ptr_cast<and_expr>(xpr))
        { // we evaluate the expression recursively..
            for (const auto &arg : and_xpr->args())
                eval(arg);
            return and_xpr->val();
        }
        else if (auto or_xpr = utils::s_ptr_cast<or_expr>(xpr))
        { // we evaluate the expression recursively..
            for (const auto &arg : or_xpr->args())
                eval(arg);
            return or_xpr->val();
        }
        throw std::runtime_error("unexpected expression type");
    }

    size_t network::add_var(std::string_view name)
    {
        if (auto it = var_map.find(name.data()); it != var_map.end())
            return it->second;
        else
        {
            size_t id = var_map.size();
            var_map.emplace(name.data(), id);
            assigns.push_back(utils::Undefined);
            watches.emplace_back();
            watches.emplace_back();
            level.emplace_back(0);
            reason.emplace_back(std::nullopt);
            return id;
        }
    }

    void network::add_term(bool_expr expr)
    {
        if (auto bool_xpr = utils::s_ptr_cast<bool_var>(expr))
            add_clause(bool_xpr); // we have a unit clause..
        else if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr))
            add_clause(not_xpr); // we have a unit clause..
        else if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr))
            add_clause(or_xpr); // we have a clause..
        else
            throw std::runtime_error("unexpected expression type");
    }

    void network::add_clause(bool_expr expr)
    {
        std::vector<utils::lit> lits;
        if (auto or_xpr = utils::s_ptr_cast<or_expr>(expr)) // we have a clause..
            for (const auto &arg : or_xpr->args())
            {
                if (auto bool_xpr = utils::s_ptr_cast<bool_var>(arg))
                    lits.push_back(utils::lit(add_var(arg->get_name()), true));
                else if (auto not_xpr = utils::s_ptr_cast<not_expr>(arg))
                    lits.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
                else
                    throw std::runtime_error("unexpected expression type");
            }
        else if (auto not_xpr = utils::s_ptr_cast<not_expr>(expr)) // we have a unit clause..
            lits.push_back(utils::lit(add_var(not_xpr->arg()->get_name()), false));
        else // we have a unit clause..
            lits.push_back(utils::lit(add_var(expr->get_name()), true));

        // we sort the clause to make sure that we can easily check for duplicates..
        std::sort(lits.begin(), lits.end(), [](const auto &l0, const auto &l1)
                  { return variable(l0) < variable(l1); });
        utils::lit p;
        size_t j = 0;
        for (auto it = lits.cbegin(); it != lits.cend(); ++it)
            if (value(*it) == utils::True || *it == !p)
                return; // the clause is already satisfied or represents a tautology..
            else if (value(*it) != utils::False && *it != p)
            { // we need to include this literal in the clause..
                p = *it;
                lits[j++] = p;
            }
        lits.resize(j);

        switch (lits.size())
        {
        case 0: // the clause is unsatisfable under the current assignment (so is the problem)..
            throw unsolvable_exception();
        case 1: // the clause is unique under the current assignment..
            if (!enqueue(lits[0]))
                throw unsolvable_exception();
            break;
        default: // we need to create a new clause..
            clauses.push_back(new clause(*this, std::move(lits)));
        }
    }

    bool network::enqueue(const utils::lit &p, const std::optional<utils::ref_wrapper<clause>> &c) noexcept
    {
        if (auto val = value(p); val != utils::Undefined)
            return val; // the literal is already assigned..
        assigns[variable(p)] = sign(p);
        level[variable(p)] = decision_level();
        if (c)
            reason[variable(p)] = c;
        trail.push_back(p);
        prop_queue.push(p);
        return true;
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

    void network::record(std::vector<utils::lit> lits) noexcept
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
            [[maybe_unused]] bool e = enqueue(l0, *c);
            assert(e);
            clauses.push_back(c);
        }
    }

    void network::pop_one() noexcept
    {
        auto v = variable(trail.back());
        assigns[v] = utils::Undefined;
        level[v] = 0;
        reason[v].reset();
        trail.pop_back();
    }

    clause::clause(network &slv, std::vector<utils::lit> &&lits) noexcept : slv(slv), lits(std::move(lits))
    {
        assert(lits.size() >= 2);
        slv.watches[index(!lits[0])].emplace_back(*this);
        slv.watches[index(!lits[1])].emplace_back(*this);
    }

    bool clause::propagate(const utils::lit &p) noexcept
    {
        assert(slv.value(p) == utils::True);
        // make sure false literal is lits[1]..
        if (variable(lits[0]) == variable(p))
            std::swap(lits[0], lits[1]);
        assert(variable(lits[1]) == variable(p));

        // if 0th watch is true, the clause is already satisfied..
        if (slv.value(lits[0]) == utils::True)
        {
            slv.watches[index(p)].emplace_back(*this);
            return true;
        }

        // we look for a new literal to watch..
        for (size_t i = 1; i < lits.size(); ++i)
            if (slv.value(lits[i]) != utils::False)
            {
                std::swap(lits[1], lits[i]);
                slv.watches[index(!lits[1])].emplace_back(*this);
                return true;
            }

        // clause is unit under assignment..
        slv.watches[index(p)].emplace_back(*this);
        return slv.enqueue(lits[0]);
    }

    bool clause::simplify() noexcept
    {
        size_t j = 0;
        for (size_t i = 0; i < lits.size(); ++i)
            switch (slv.value(lits[i]))
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
            assert(slv.value(lits[i]) == utils::False); // when this function is called, the clause is unit..
            r.push_back(!lits[i]);
        }
        return r;
    }
} // namespace semitone
