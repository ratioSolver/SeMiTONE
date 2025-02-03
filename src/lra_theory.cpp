#include "lra_theory.hpp"
#include "network.hpp"
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace semitone
{
    lra_theory::lra_theory(network &slv) noexcept : theory(slv) {}

    utils::var lra_theory::add_var(std::string_view name, const utils::rational &lb, const utils::rational &ub) noexcept
    {
        if (auto it = var_map.find(name.data()); it != var_map.end())
            return it->second;
        else
        {
            utils::var id = var_map.size();
            var_map.emplace(name.data(), id);
            c_bounds.push_back({utils::inf_rational(lb), {}});
            c_bounds.push_back({utils::inf_rational(ub), {}});
            return id;
        }
    }

    utils::lit lra_theory::add_constraint(bool_expr expr, bool bind)
    {
        unsigned char op;
        utils::lin l;
        if (auto real_lt_xpr = utils::s_ptr_cast<real_lt>(expr))
        {
            op = 0;
            l = linearize(real_lt_xpr->left()) - linearize(real_lt_xpr->right());
        }
        else if (auto real_le_xpr = utils::s_ptr_cast<real_le>(expr))
        {
            op = 1;
            l = linearize(real_le_xpr->left()) - linearize(real_le_xpr->right());
        }
        else if (auto real_eq_xpr = utils::s_ptr_cast<real_eq>(expr))
        {
            op = 2;
            l = linearize(real_eq_xpr->left()) - linearize(real_eq_xpr->right());
        }
        else if (auto real_ge_xpr = utils::s_ptr_cast<real_ge>(expr))
        {
            op = 3;
            l = linearize(real_ge_xpr->right()) - linearize(real_ge_xpr->left());
        }
        else if (auto real_gt_xpr = utils::s_ptr_cast<real_gt>(expr))
        {
            op = 4;
            l = linearize(real_gt_xpr->right()) - linearize(real_gt_xpr->left());
        }
        else
            throw std::runtime_error("unexpected expression type");

        // we remove the basic variables from the expression and replace them with their corresponding linear expressions in the tableau..
        std::vector<utils::var> vars;
        vars.reserve(l.vars.size());
        std::transform(l.vars.begin(), l.vars.end(), std::back_inserter(vars), [](const auto &pair)
                       { return pair.first; });
        for (const auto &v : vars)
            if (tableau.find(v) != tableau.cend())
            {
                auto c = l.vars.at(v);
                l.vars.erase(v);
                l += c * tableau.at(v)->l;
            }

        switch (l.vars.size())
        {
        case 0: // the constraint is a tautology..
            switch (op)
            {
            case 0: // `<`
                return utils::rational::zero < l.known_term ? utils::TRUE_lit : utils::FALSE_lit;
            case 1: // `<=`
                return utils::rational::zero <= l.known_term ? utils::TRUE_lit : utils::FALSE_lit;
            case 2: // `=`
                return utils::rational::zero == l.known_term ? utils::TRUE_lit : utils::FALSE_lit;
            case 3: // `>=`
                return utils::rational::zero >= l.known_term ? utils::TRUE_lit : utils::FALSE_lit;
            case 4: // `>`
                return utils::rational::zero > l.known_term ? utils::TRUE_lit : utils::FALSE_lit;
            }
            break;
        case 1:
        { // the expression is an inequality with a single variable..
            const auto [v, c] = *l.vars.cbegin();
            switch (op)
            {
            case 0: // `<`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term, -1) / c;
                if (c > 0)
                    new_leq(v, c_right, bind);
                else
                    new_geq(v, c_right, bind);
            }
            break;
            case 1: // `<=`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term) / c;
                if (c > 0)
                    new_leq(v, c_right, bind);
                else
                    new_geq(v, c_right, bind);
            }
            break;
            case 2: // `==`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term) / c;
                new_leq(v, c_right, bind);
                new_geq(v, c_right, bind);
            }
            break;
            case 3: // `>=`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term) / c;
                if (c > 0)
                    new_geq(v, c_right, bind);
                else
                    new_leq(v, c_right, bind);
            }
            break;
            case 4: // `>`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term, 1) / c;
                if (c > 0)
                    new_geq(v, c_right, bind);
                else
                    new_leq(v, c_right, bind);
            }
            break;
            }
            break;
        }
        default: // the expression is an inequality with multiple variables
            switch (op)
            {
            case 0: // `<`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term, -1);
                l.known_term = utils::rational::zero;
                new_leq(add_var(std::move(l)), c_right, bind);
            }
            break;
            case 1: // `<=`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term);
                l.known_term = utils::rational::zero;
                new_leq(add_var(std::move(l)), c_right, bind);
            }
            break;
            case 2: // `==`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term);
                l.known_term = utils::rational::zero;
                new_leq(add_var(std::move(l)), c_right, bind);
                new_geq(add_var(std::move(l)), c_right, bind);
            }
            break;
            case 3: // `>=`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term);
                l.known_term = utils::rational::zero;
                new_geq(add_var(std::move(l)), c_right, bind);
            }
            break;
            case 4: // `>`
            {
                const utils::inf_rational c_right = utils::inf_rational(-l.known_term, 1);
                l.known_term = utils::rational::zero;
                new_geq(add_var(std::move(l)), c_right, bind);
            }
            break;
            }
        }
    }

    utils::lin lra_theory::linearize(real_expr expr)
    {
        if (auto rv = utils::s_ptr_cast<real_var>(expr))
            return utils::lin(add_var(rv->get_name(), rv->lb(), rv->ub()), utils::rational::one);
        else if (auto rc = utils::s_ptr_cast<real_const>(expr))
            return utils::lin(utils::rational(rc->val()));
        else if (auto rs = utils::s_ptr_cast<real_sum>(expr))
        {
            utils::lin l;
            for (const auto &arg : rs->args())
                l += linearize(arg);
            return l;
        }
        else if (auto rs = utils::s_ptr_cast<real_sub>(expr))
        {
            utils::lin l;
            for (const auto &arg : rs->args())
                l -= linearize(arg);
            return l;
        }
        else if (auto rm = utils::s_ptr_cast<real_mul>(expr))
        {
            utils::lin l(utils::rational::one);
            for (const auto &arg : rm->args())
            {
                auto lin = linearize(arg);
                if (lin.vars.empty())
                    l *= lin.known_term;
                else
                {
                    assert(l.vars.empty());
                    l = lin * l.known_term;
                }
            }
            return l;
        }
        else if (auto rd = utils::s_ptr_cast<real_div>(expr))
        {
            utils::lin l;
            for (const auto &arg : rd->args())
            {
                auto lin = linearize(arg);
                if (lin.vars.empty())
                    l /= lin.known_term;
                else
                {
                    assert(l.vars.empty());
                    l = lin / l.known_term;
                }
            }
            return l;
        }
        else
            throw std::runtime_error("unexpected expression type");
    }

    utils::var lra_theory::add_var(const utils::lin &&l) noexcept
    { // we create, if needed, a new arithmetic variable which is equal to the given linear expression..
        assert(!l.vars.empty());
        const auto s_expr = to_string(l);
        if (const auto it = var_map.find(s_expr); it != var_map.cend())
            return it->second;

        const auto slack = add_var("slk" + std::to_string(tableau.size()));

        utils::inf_rational val(l.known_term), lb(l.known_term), ub(l.known_term);
        std::vector<utils::lit> lb_reason, ub_reason;
        for (const auto &[v, c] : l.vars)
        {
            val += c * vals[v];
            lb += (is_positive(c) ? c_bounds[lb_index(v)].value : c_bounds[ub_index(v)].value) * c;
            lb_reason.insert(lb_reason.end(), c_bounds[lb_index(v)].reason.cbegin(), c_bounds[lb_index(v)].reason.cend());
            ub += (is_positive(c) ? c_bounds[ub_index(v)].value : c_bounds[lb_index(v)].value) * c;
            ub_reason.insert(ub_reason.end(), c_bounds[ub_index(v)].reason.cbegin(), c_bounds[ub_index(v)].reason.cend());
        }

        c_bounds[lb_index(slack)] = {lb, lb_reason}; // we set the lower bound of the slack variable to the lower bound of the linear expression
        c_bounds[ub_index(slack)] = {ub, ub_reason}; // we set the upper bound of the slack variable to the upper bound of the linear expression
        vals[slack] = val;                           // we set the value of the slack variable to the value of the linear expression
        var_map.emplace(s_expr, slack);              // we add the linear expression to the expressions
        new_row(slack, std::move(l));                // we add the new row `slack = ...` to the tableau
        return slack;
    }

    void lra_theory::new_row(const utils::var x_i, const utils::lin &&xpr) noexcept
    {
        assert(tableau.find(x_i) == tableau.cend()); // the variable `x_i` must not be in the tableau..
        for (const auto &x : xpr.vars)
            t_watches[x.first].insert(x_i);
        tableau.emplace(x_i, utils::make_u_ptr<lra_eq>(x_i, std::move(xpr)));
    }

    void lra_theory::new_leq(const utils::var x, const utils::inf_rational &v, bool bind) noexcept {}

    void lra_theory::new_geq(const utils::var x, const utils::inf_rational &v, bool bind) noexcept {}
} // namespace semitone
