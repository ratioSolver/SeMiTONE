#include "theory.hpp"
#include "network.hpp"
#include <cassert>

namespace semitone
{
    theory::theory(network &net) noexcept : net(net) {}

    void theory::bind(const utils::var &v) noexcept
    {
        assert(net.value(v) == utils::Undefined);
        net.binds[v].emplace(this);
    }

    void theory::analyze_and_backjump() noexcept
    {
        // we create a conflict clause for the analysis..
        clause cnfl_cl(net, std::move(cnfl));

        // .. and we analyze the conflict..
        std::vector<utils::lit> no_good;
        size_t bt_level = 0;
        net.analyze(cnfl_cl, no_good, bt_level);

        // we backjump..
        while (net.decision_level() > bt_level)
            net.pop();
        // .. and record the no-good..
        net.record(std::move(no_good));
    }
} // namespace semitone