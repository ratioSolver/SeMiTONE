#include "la_theory.hpp"

namespace semitone
{
    la_theory::la_theory(network &slv) noexcept : theory(slv) {}

    void la_theory::push() noexcept
    {
    }

    void la_theory::pop() noexcept
    {
    }

    size_t la_theory::add_real(std::string_view name, const utils::inf_rational &lb, const utils::inf_rational &ub)
    {
        if (auto it = var_map.find(name.data()); it != var_map.end())
            return it->second;
        else
        {
            size_t id = var_map.size();
            var_map.emplace(name.data(), id);
            c_bounds.emplace_back(bound{lb, {}}); // add the lower bound..
            c_bounds.emplace_back(bound{ub, {}}); // add the upper bound..
            vals.push_back(utils::inf_rational(utils::rational::zero));
            return id;
        }
    }
} // namespace semitone