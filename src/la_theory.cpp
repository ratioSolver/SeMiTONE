#include "la_theory.hpp"
#include "network.hpp"
#include "logging.hpp"

namespace semitone
{
    la_theory::la_theory(network &slv) noexcept : theory(slv) {}

    void la_theory::add(bool_expr expr)
    {
        LOG_DEBUG("adding linear arithmetic constraint: " << expr->get_name());
        if (auto int_lt_xpr = utils::s_ptr_cast<int_lt>(expr))
        {
            utils::lin lhs = to_lin(int_lt_xpr->left());
            utils::integer rhs = utils::s_ptr_cast<int_const>(int_lt_xpr->right())->val();
            switch (lhs.vars.size())
            {
            case 0: // the expression is a constant
                if (lhs.known_term >= rhs.value())
                    throw unsolvable_exception();
                break;
            }
        }
    }

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

    utils::lin la_theory::to_lin(int_expr expr)
    {
        utils::lin xpr;
        for (const auto &arg : utils::s_ptr_cast<int_sum>(expr)->args())
            if (auto int_mul_xpr = utils::s_ptr_cast<int_mul>(arg))
            {
                auto var = add_real(utils::s_ptr_cast<int_var>(int_mul_xpr->args()[0])->get_name());
                auto coef = utils::s_ptr_cast<int_const>(int_mul_xpr->args()[1])->val().value();
                if (auto it = tableau.find(var); it != tableau.cend())
                    xpr += utils::rational(coef) * it->second->l;
            }
            else
                throw std::runtime_error("unexpected expression type");
        return xpr;
    }
    utils::lin la_theory::to_lin(real_expr expr)
    {
        utils::lin xpr;
        for (const auto &arg : utils::s_ptr_cast<real_sum>(expr)->args())
            if (auto real_mul_xpr = utils::s_ptr_cast<real_mul>(arg))
            {
                auto var = add_real(utils::s_ptr_cast<real_var>(real_mul_xpr->args()[0])->get_name());
                auto coef = utils::s_ptr_cast<real_const>(real_mul_xpr->args()[1])->val();
                if (tableau.find(var) != tableau.cend())
                    xpr += coef * tableau.at(var)->l;
            }
            else
                throw std::runtime_error("unexpected expression type");
        return xpr;
    }
} // namespace semitone