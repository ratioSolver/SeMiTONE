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
} // namespace semitone