#include "lra_theory.hpp"

namespace semitone
{
    lra_theory::lra_theory(std::shared_ptr<sat_core> sat) : theory(sat) {}

    VARIABLE_TYPE lra_theory::new_var() noexcept
    {
        auto var = vals.size();
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::negative_infinite), utils::TRUE_lit});
        c_bounds.emplace_back(bound{utils::inf_rational(utils::rational::positive_infinite), utils::TRUE_lit});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        return var;
    }
} // namespace semitone