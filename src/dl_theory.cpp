#include "dl_theory.hpp"
#include "semitone.hpp"
#include "logging.hpp"
#include <cassert>

#ifdef BUILD_LISTENERS
#define VAR_CHANGED(v)                                               \
    if (auto it = var_listeners.find(v); it != var_listeners.cend()) \
        for (const auto &l : it->second)                             \
            l->on_tp_change(v);
#else
#define VAR_CHANGED(v)
#endif

namespace smt
{
    dl_theory::dl_theory(semitone &net, const size_t &size) noexcept : theory(net), dists(size, std::vector<utils::rational>(size, utils::rational(utils::rational::positive_infinite))), preds(size, std::vector<utils::var>(size))
    {
        assert(size > 1);
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = utils::rational(utils::rational::zero);
            std::fill(preds[i].begin(), preds[i].end(), std::numeric_limits<utils::var>::max());
            preds[i][i] = i;
        }
    }

    utils::var dl_theory::new_var() noexcept
    {
        auto var = n_vars++;
        if (var >= dists.size())
            resize((dists.size() * 3) / 2 + 1);
        return var;
    }

    void dl_theory::new_distance(utils::var from, utils::var to, const utils::rational &dist, const utils::lit &p)
    {
        assert(net.value(p) != utils::False);
        if (dists[to][from] < -dist)
            return net.add_clause({!p}); // the constraint is conflicting..
        if (dists[from][to] <= dist)
            return; // the constraint is redundant..

        LOG_TRACE("[" << to_string(p) << "] tp" << std::to_string(from) << " -> tp" << std::to_string(to) << " : " << to_string(dist));
        if (net.value(p) == utils::True) // we update the distance..
            set_dist(from, to, dist);
        else
        { // we add the constraint to the list of constraints..
            bind(variable(p));
            auto constr = new distance_constraint(p, from, to, dist);
            dist_constrs[{from, to}].emplace_back(*constr);
            var_constrs[variable(p)].emplace_back(constr);
        }
    }

    bool dl_theory::propagate(const utils::lit &p) noexcept
    {
        assert(var_constrs.count(variable(p)));
        if (net.value(variable(p)) == utils::True)
        {
            auto &constrs = var_constrs.at(variable(p));
            for (const auto &constr : constrs)
                if (dists[constr->get_to()][constr->get_from()] < -constr->get_dist())
                { // the constraint is inconsistent, we have a conflict..
                    assert(cnfl.empty());
                    cnfl.emplace_back(!constr->get_lit());
                    utils::var c_to = constr->get_from();
                    while (c_to != constr->get_to())
                    {
                        const auto &c_d = *dist_constr.find({preds[constr->get_to()][c_to], c_to})->second;
                        switch (net.value(c_d.get_lit()))
                        {
                        case utils::True:
                            cnfl.emplace_back(!c_d.get_lit());
                            break;
                        case utils::False:
                            cnfl.emplace_back(c_d.get_lit());
                            break;
                        }
                        c_to = preds[constr->get_to()][c_to];
                    }
                    return false;
                }
                else if (dists[constr->get_from()][constr->get_to()] > constr->get_dist())
                { // the constraint is not trivially satisfied
                    const auto from_to = std::make_pair(constr->get_from(), constr->get_to());
                    if (!layers.empty() && !layers.back().old_constrs.count(from_to))
                    {
                        if (const auto &c_dist = dist_constr.find(from_to); c_dist != dist_constr.cend()) // we store the current constraint for backtracking purposes..
                            layers.back().old_constrs.emplace(c_dist->first, c_dist->second);
                        else // we store the absence of a constraint for backtracking purposes..
                            layers.back().old_constrs.emplace(from_to, std::nullopt);
                    }
                    dist_constr.emplace(from_to, *constr);
                    propagate(constr->get_from(), constr->get_to(), constr->get_dist());
                }
        }
        return true;
    }

    void dl_theory::propagate(utils::var from, utils::var to, const utils::rational &dist) noexcept
    {
        LOG_TRACE("tp" << from << " -> tp" << to << " : " << to_string(dist));
        assert(!is_infinite(dist));
        set_dist(from, to, dist);
        set_pred(from, to, from);
        std::vector<utils::var> set_i;
        std::vector<utils::var> set_j;
        std::vector<std::pair<utils::var, utils::var>> c_updates;
        c_updates.emplace_back(from, to);
        c_updates.emplace_back(to, from);

        // we start with an O(n) loop..
        for (size_t u = 0; u < n_vars; ++u)
        {
            if (dists[u][from] < dists[u][to] - dist)
            { // u -> from -> to is shorter than u -> to..
                set_dist(u, to, dists[u][from] + dist);
                set_pred(u, to, preds[from][to]);
                set_i.emplace_back(u);
                c_updates.emplace_back(u, to);
                c_updates.emplace_back(to, u);
            }
            if (dists[to][u] < dists[from][u] - dist)
            { // from -> to -> u is shorter than from -> u..
                set_dist(from, u, dists[to][u] + dist);
                set_pred(from, u, preds[to][u]);
                set_j.emplace_back(u);
                c_updates.emplace_back(from, u);
                c_updates.emplace_back(u, from);
            }
        }

        // finally, we loop over set_i and set_j in O(n^2) time (but possibly much less)..
        for (const auto &i : set_i)
            for (const auto &j : set_j)
                if (i != j && dists[i][to] + dists[to][j] < dists[i][j])
                { // i -> from -> to -> j is shorter than i -> j--
                    set_dist(i, j, dists[i][to] + dists[to][j]);
                    set_pred(i, j, preds[to][j]);
                    c_updates.emplace_back(i, j);
                    c_updates.emplace_back(j, i);
                }

        for (const auto &c_pairs : c_updates)
            if (const auto &c_dists = dist_constrs.find(c_pairs); c_dists != dist_constrs.cend())
                for (const auto &c_dist : c_dists->second)
                    if (net.value(c_dist->get_lit()) == utils::Undefined && dists[c_dist->get_to()][c_dist->get_from()] < -c_dist->get_dist())
                    { // the constraint is inconsistent..
                        std::vector<utils::lit> cnfl;
                        cnfl.emplace_back(!c_dist->get_lit());
                        utils::var c_to = c_dist->get_from();
                        while (c_to != c_dist->get_to())
                        {
                            const auto &c_d = *dist_constr.find({preds[c_dist->get_to()][c_to], c_to})->second;
                            switch (net.value(c_d.get_lit()))
                            {
                            case utils::True:
                                cnfl.emplace_back(!c_d.get_lit());
                                break;
                            case utils::False:
                                cnfl.emplace_back(c_d.get_lit());
                                break;
                            }
                            c_to = preds[c_dist->get_to()][c_to];
                        }
                        // we propagate the reason for assigning false to dist->b..
                        record(std::move(cnfl));
                    }
    }

    bool dl_theory::check() noexcept { return true; }

    void dl_theory::push() noexcept { layers.emplace_back(); }

    void dl_theory::pop() noexcept
    {
        for (const auto &[vars, dist] : layers.back().old_dists)
            dists[vars.first][vars.second] = dist;
        for (const auto &[vars, pred] : layers.back().old_preds)
            preds[vars.first][vars.second] = pred;
        for (const auto &[vars, dist] : layers.back().old_constrs)
            if (dist.has_value()) // we replace the current constraint..
                dist_constr.emplace(vars, *dist);
            else // we make some cleanings..
                dist_constr.erase(vars);
        layers.pop_back();
    }

    void dl_theory::set_dist(utils::var from, utils::var to, const utils::rational &dist) noexcept
    {
        assert(dists[from][to] > dist);                                                 // we should never increase the distance
        if (!layers.empty() && !layers.back().old_dists.count({from, to}))              // we have not updated this distance yet
            layers.back().old_dists.emplace(std::make_pair(from, to), dists[from][to]); // save the old distance
        dists[from][to] = dist;                                                         // set the new distance
        if (from == 0)
        { // we have set a distance from the source
            VAR_CHANGED(to);
        }
        if (to == 0)
        { // we have set a distance to the source
            VAR_CHANGED(from);
        }
    }

    void dl_theory::set_pred(utils::var from, utils::var to, utils::var pred) noexcept
    {
        assert(dist_constr.find({pred, to}) != dist_constr.end());
        assert(preds[from][to] != pred);                                                // we should never set the same predecessor
        if (!layers.empty() && !layers.back().old_preds.count({from, to}))              // we have not updated this predecessor yet
            layers.back().old_preds.emplace(std::make_pair(from, to), preds[from][to]); // save the old predecessor
        preds[from][to] = pred;                                                         // set the new predecessor
    }

    void dl_theory::resize(const size_t &size) noexcept
    {
        const size_t c_size = dists.size();
        for (size_t i = 0; i < c_size; ++i)
        {
            dists[i].resize(size, utils::rational(utils::rational::positive_infinite));
            preds[i].resize(size, std::numeric_limits<INT_TYPE>::max());
        }
        dists.resize(size, std::vector<utils::rational>(size, utils::rational(utils::rational::positive_infinite)));
        preds.resize(size, std::vector<utils::var>(size, std::numeric_limits<INT_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = utils::rational(utils::rational::zero);
            preds[i][i] = i;
        }
    }

    [[nodiscard]] std::ostream &operator<<(std::ostream &os, const dl_theory &th)
    {
        os << "Distance matrix:\n";
        for (size_t i = 0; i < th.dists.size(); ++i)
        {
            for (size_t j = 0; j < th.dists.size(); ++j)
                os << to_string(th.dists[i][j]) << " ";
            os << '\n';
        }
        os << "Predecessor matrix:\n";
        for (size_t i = 0; i < th.preds.size(); ++i)
        {
            for (size_t j = 0; j < th.preds.size(); ++j)
                os << std::to_string(th.preds[i][j]) << " ";
            os << '\n';
        }
        return os;
    }
} // namespace semitone