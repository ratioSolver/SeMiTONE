#include "ov_theory.hpp"

namespace semitone
{
    ov_theory::ov_theory(std::shared_ptr<sat_core> sat) : theory(sat) {}

    VARIABLE_TYPE ov_theory::new_var(std::vector<std::reference_wrapper<utils::enum_val>> &&domain, const bool enforce_exct_one) noexcept
    {
    }
} // namespace semitone
