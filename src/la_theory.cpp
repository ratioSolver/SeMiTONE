#include "la_theory.hpp"
#include "network.hpp"
#include "logging.hpp"
#include <cassert>

namespace semitone
{
    la_theory::la_theory(network &slv) noexcept : theory(slv) {}

    utils::var la_theory::new_int(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        utils::var var = vals.size();
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    utils::var la_theory::new_real(const utils::inf_rational &lb, const utils::inf_rational &ub) noexcept
    {
        assert(lb < ub);
        utils::var var = vals.size();
        c_bounds.emplace_back(bound{lb, {}});
        c_bounds.emplace_back(bound{ub, {}});
        vals.push_back(utils::inf_rational(utils::rational::zero));
        a_watches.emplace_back();
        t_watches.emplace_back();
        return var;
    }

    void la_theory::push() noexcept
    {
    }

    void la_theory::pop() noexcept
    {
    }
} // namespace semitone