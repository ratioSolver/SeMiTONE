#include "dl_theory.hpp"

namespace semitone
{
    dl_theory::dl_theory(network &net) noexcept : theory(net) {}

    bool dl_theory::propagate(const utils::lit &p) noexcept
    {
        return false;
    }

    bool dl_theory::check() noexcept
    {
        return false;
    }

    void dl_theory::push() noexcept {}

    void dl_theory::pop() noexcept {}
} // namespace semitone