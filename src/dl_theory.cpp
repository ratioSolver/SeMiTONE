#include "dl_theory.hpp"
#include "network.hpp"
#include "logging.hpp"
#include <cassert>

namespace semitone
{
    dl_theory::dl_theory(network &net, const size_t &size) noexcept : theory(net), dists(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite))), preds(size, std::vector<utils::var>(size))
    {
        assert(size > 1);
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
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

    void dl_theory::add_distance(utils::var from, utils::var to, const utils::inf_rational &dist)
    {
        if (dists[to][from] < -dist)
            throw unsolvable_exception(); // the problem is unsolvable..
        if (dists[from][to] <= dist)
            return; // the constraint is redundant..
        set_dist(from, to, dist);
    }

    void dl_theory::new_distance(utils::lit &b, utils::var from, utils::var to, const utils::inf_rational &dist) noexcept
    {
        // TODO: implement this function..
    }

    bool dl_theory::propagate(const utils::lit &p) noexcept
    {
        return true;
    }

    bool dl_theory::check() noexcept
    {
        return true;
    }

    void dl_theory::push() noexcept {}

    void dl_theory::pop() noexcept {}

    void dl_theory::set_dist(utils::var from, utils::var to, const utils::inf_rational &dist) noexcept
    {
        assert(dists[from][to] > dist);                                                 // we should never increase the distance
        if (!layers.empty() && !layers.back().old_dists.count({from, to}))              // we have not updated this distance yet
            layers.back().old_dists.emplace(std::make_pair(from, to), dists[from][to]); // save the old distance
        dists[from][to] = dist;                                                         // set the new distance
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
            dists[i].resize(size, utils::inf_rational(utils::rational::positive_infinite));
            preds[i].resize(size, std::numeric_limits<INT_TYPE>::max());
        }
        dists.resize(size, std::vector<utils::inf_rational>(size, utils::inf_rational(utils::rational::positive_infinite)));
        preds.resize(size, std::vector<utils::var>(size, std::numeric_limits<INT_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = utils::inf_rational(utils::rational::zero);
            preds[i][i] = i;
        }
    }
} // namespace semitone