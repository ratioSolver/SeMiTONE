#include "sat_core.h"
#include "clause.h"
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
        watches.emplace_back();
        watches.emplace_back();
        assigns.emplace_back(Undefined);
        level.emplace_back(0);
        return id;
    }

    nlohmann::json sat_core::to_json() const noexcept
    {
        nlohmann::json j_sat;

        nlohmann::json::array_t j_cnstrs;
        for (const auto &c : constrs)
            j_cnstrs.push_back(c->to_json());

        j_sat["constrs"] = j_cnstrs;
        return j_sat;
    }
} // namespace semitone