#include "sat_stack.h"
#include "theory.h"
#include "sat_value_listener.h"
#include <cassert>

namespace semitone
{
    SEMITONE_EXPORT sat_stack::sat_stack() { stack.push_back(new sat_core()); }

    SEMITONE_EXPORT void sat_stack::push() noexcept
    {
        stack.push_back(new sat_core(*stack.back()));
        for (auto &th : stack.back()->theories)
        {
            th->sat = stack.back();
            th->push();
        }
        for (auto &l : stack.back()->listeners)
            l->sat = stack.back();
    }

    SEMITONE_EXPORT void sat_stack::pop() noexcept
    {
        assert(stack.size() > 1);
        std::vector<lit> trail = stack.back()->trail;
        stack.pop_back();
        for (auto &th : stack.back()->theories)
        {
            th->sat = stack.back();
            th->pop();
        }
        for (auto &l : stack.back()->listeners)
            l->sat = stack.back();

        for (const auto &p : trail)
            if (const auto at_p = stack.back()->listening.find(variable(p)); at_p != stack.back()->listening.cend())
                for (const auto &l : at_p->second)
                    l->sat_value_change(variable(p));
    }
} // namespace semitone