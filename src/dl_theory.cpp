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