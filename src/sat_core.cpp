#include "sat_core.h"
#include <cassert>

namespace semitone
{
    SEMITONE_EXPORT sat_core::sat_core()
    {
        [[maybe_unused]] var c_false = new_var(); // the false constant..
        assert(c_false == FALSE_var);
        assigns[FALSE_var] = False;
        level[FALSE_var] = 0;
    }
    SEMITONE_EXPORT sat_core::~sat_core() {}

    SEMITONE_EXPORT var sat_core::new_var() noexcept
    {
        const auto id = assigns.size();
        assigns.emplace_back(Undefined);
        level.emplace_back(0);
        return id;
    }
} // namespace semitone