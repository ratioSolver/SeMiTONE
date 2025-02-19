#include "theory.hpp"
#include "semitone.hpp"
#include <cassert>

namespace smt
{
    theory::theory(semitone &net) noexcept : net(net) {}

    void theory::bind(const utils::var &v) noexcept
    {
        assert(net.value(v) == utils::Undefined);
        net.binds[v].emplace(this);
    }

    void theory::record(std::vector<utils::lit> &&clause) noexcept { net.record(std::move(clause)); }
} // namespace semitone