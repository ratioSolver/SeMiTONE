#include <cassert>
#include "sat_core.hpp"
#include "theory.hpp"

namespace semitone
{
    sat_core::sat_core()
    {
        [[maybe_unused]] VARIABLE_TYPE c_false = new_var(); // the false constant..
        assert(c_false == FALSE_var);
        assigns[FALSE_var] = utils::False;
        level[FALSE_var] = 0;
    }

    VARIABLE_TYPE sat_core::new_var() noexcept
    {
        const VARIABLE_TYPE x = assigns.size();
        assigns.push_back(utils::Undefined);
        watches.emplace_back();
        watches.emplace_back();
        // exprs.emplace("b" + std::to_string(id), id);
        level.emplace_back(0);
        reason.emplace_back(std::nullopt);
        return x;
    }

    bool sat_core::enqueue(const lit &p, std::optional<std::reference_wrapper<constr>> c) noexcept
    {
        if (auto val = value(p); val != utils::Undefined)
            return val;
        assigns[variable(p)] = sign(p);
        level[variable(p)] = decision_level();
        if (c)
            reason[variable(p)] = c;
        trail.push_back(p);
        prop_queue.push(p);
        return true;
    }
} // namespace semitone
