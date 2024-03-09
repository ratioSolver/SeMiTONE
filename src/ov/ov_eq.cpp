#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "ov_eq.hpp"
#include "sat_core.hpp"
#include "ov_theory.hpp"

namespace semitone
{
    ov_eq::ov_eq(ov_theory &ov, const VARIABLE_TYPE left, const VARIABLE_TYPE right, const utils::lit &ctr) noexcept : constr(ov.get_sat()), ov(ov), left(left), right(right), ctr(ctr)
    {
        assert(ov.get_sat().root_level());
        assert(value(ctr) == utils::Undefined);
        watches(ctr).push_back(*this);
        watches(!ctr).push_back(*this);
        for (const auto &v : ov.domains[left])
        {
            left_domain.emplace(variable(v.second), v.first.get());
            left_domain_set.emplace(&v.first.get(), v.second);
            watches(v.second).push_back(*this);
            watches(!v.second).push_back(*this);
        }
        for (const auto &v : ov.domains[right])
        {
            right_domain.emplace(variable(v.second), v.first.get());
            right_domain_set.emplace(&v.first.get(), v.second);
            watches(v.second).push_back(*this);
            watches(!v.second).push_back(*this);
        }
    }

    std::unique_ptr<constr> ov_eq::copy(sat_core &) noexcept { return std::make_unique<ov_eq>(ov, left, right, ctr); }

    bool ov_eq::propagate(const utils::lit &p) noexcept
    {
        assert(value(p) == utils::True);
        watches(p).emplace_back(*this);
        if (!must_propagate(p))
            return true; // the literal is already propagated by this constraint

        if (variable(p) == variable(ctr))
        { // the control variable is assigned
            if (value(ctr) == utils::True)
            { // the equality is `true`
                // we propagate the values that are in the intersection and prune the values that are not in the intersection
                for (const auto &[v, l] : left_domain_set)
                    if (auto it = right_domain_set.find(v); it == right_domain_set.end())
                        switch (value(l))
                        {
                        case utils::True:
                            if (!enqueue(it->second))
                                return false;
                            break;
                        case utils::False:
                            if (!enqueue(!it->second))
                                return false;
                            break;
                        }
                    else if (!enqueue(!l))
                        return false;
                for (const auto &[v, l] : right_domain_set)
                    if (auto it = left_domain_set.find(v); it == left_domain_set.end())
                        switch (value(l))
                        {
                        case utils::True:
                            if (!enqueue(it->second))
                                return false;
                            break;
                        case utils::False:
                            if (!enqueue(!it->second))
                                return false;
                            break;
                        }
                    else if (!enqueue(!l))
                        return false;
            }
            else
            { // the equality is `false`
                auto left = std::find_if(left_domain.begin(), left_domain.end(), [this](const auto &pair)
                                         { return value(pair.first) == utils::True; });
                if (left != left_domain.end())                                                                                                // the left variable is assigned
                    if (auto right = right_domain_set.find(&left->second.get()); right != right_domain_set.end() && !enqueue(!right->second)) // we forbid the same value in the right variable
                        return false;
                auto right = std::find_if(right_domain.begin(), right_domain.end(), [this](const auto &pair)
                                          { return value(pair.first) == utils::True; });
                if (right != right_domain.end())                                                                                          // the right variable is assigned
                    if (auto left = left_domain_set.find(&right->second.get()); left != left_domain_set.end() && !enqueue(!left->second)) // we forbid the same value in the left variable
                        return false;
            }
        }
        else
        {
            const auto &v = variable(p);
            auto left_it = left_domain.find(v);
            auto right_it = right_domain.find(v);
            switch (value(ctr))
            {
            case utils::True:
                if (left_it != left_domain.end())
                    if (auto it = right_domain_set.find(&left_it->second.get()); it != right_domain_set.end())
                    {
                        if (value(left_domain_set.at(&left_it->second.get())) == utils::True)
                        {
                            if (!enqueue(it->second))
                                return false;
                        }
                        else if (!enqueue(!it->second))
                            return false;
                    }
                if (right_it != right_domain.end())
                    if (auto it = left_domain_set.find(&right_it->second.get()); it != left_domain_set.end())
                    {
                        if (value(right_domain_set.at(&right_it->second.get())) == utils::True)
                        {
                            if (!enqueue(it->second))
                                return false;
                        }
                        else if (!enqueue(!it->second))
                            return false;
                    }
                break;
            case utils::False:
                if (left_it != left_domain.end())
                    if (auto it = right_domain_set.find(&left_it->second.get()); it != right_domain_set.end() && value(left_domain_set.at(&left_it->second.get())) == utils::True && !enqueue(!it->second))
                        return false;
                if (right_it != right_domain.end())
                    if (auto it = left_domain_set.find(&right_it->second.get()); it != left_domain_set.end() && value(right_domain_set.at(&right_it->second.get())) == utils::True && !enqueue(!it->second))
                        return false;
                break;
            default:
                if (left_it != left_domain.end())
                {
                    if (auto it = right_domain_set.find(&left_it->second.get()); it != right_domain_set.end())
                    {
                        if (value(left_domain_set.at(&left_it->second.get())) == utils::True && value(it->second) == utils::True && !enqueue(ctr))
                            return false;
                    }
                    else if (value(left_domain_set.at(&left_it->second.get())) == utils::True && !enqueue(!ctr))
                        return false;
                }
                if (right_it != right_domain.end())
                {
                    if (auto it = left_domain_set.find(&right_it->second.get()); it != left_domain_set.end())
                    {
                        if (value(right_domain_set.at(&right_it->second.get())) == utils::True && value(it->second) == utils::True && !enqueue(ctr))
                            return false;
                    }
                    else if (value(right_domain_set.at(&right_it->second.get())) == utils::True && !enqueue(!ctr))
                        return false;
                }
            }
        }
        return true;
    }

    bool ov_eq::simplify() noexcept { return value(ctr) != utils::Undefined; }

    std::vector<utils::lit> ov_eq::get_reason(const utils::lit &p) const noexcept
    {
        if (is_undefined(p))
        {
            std::vector<utils::lit> reason;
            reason.reserve(left_domain.size() + right_domain.size() + 1);
            for (const auto &[v, l] : left_domain_set)
                if (value(l) != utils::Undefined)
                    reason.push_back(l);
            for (const auto &[v, l] : right_domain_set)
                if (value(l) != utils::Undefined)
                    reason.push_back(l);
            if (value(ctr) != utils::Undefined)
                reason.push_back(ctr);
            return reason;
        }
        else
        {
            const auto &v = variable(p);
            if (v == variable(ctr))
            {
                auto left_it = std::find_if(left_domain.begin(), left_domain.end(), [this](const auto &pair)
                                            { return value(pair.first) == utils::True; });
                auto right_it = std::find_if(right_domain.begin(), right_domain.end(), [this](const auto &pair)
                                             { return value(pair.first) == utils::True; });
                assert(left_it != left_domain.end() && right_it != right_domain.end());
                if (value(ctr) == utils::True)
                { // the control variable is `true`
                    assert(&left_it->second.get() == &right_it->second.get());
                    return {left_domain_set.at(&left_it->second.get()), right_domain_set.at(&right_it->second.get())};
                }
                else
                { // the control variable is `false`
                    assert(&left_it->second.get() != &right_it->second.get());
                    return {left_domain_set.at(&left_it->second.get()), !right_domain_set.at(&right_it->second.get())};
                }
            }
            else
            {
                // TOTO: implement the case where the literal is a domain value
            }
        }
        return {};
    }

    json::json ov_eq::to_json() const noexcept
    {
        json::json j_eq;
        json::json j_left{{"var", std::to_string(left)}};
        json::json j_left_domain(json::json_type::array);
        for (const auto &[v, l] : left_domain_set)
        {
            json::json j_lit{{"lit", to_string(l)}, {"val", std::to_string(reinterpret_cast<uintptr_t>(&v))}};
            switch (value(l))
            {
            case utils::True:
                j_lit["value"] = "true";
                break;
            case utils::False:
                j_lit["value"] = "false";
                break;
            }
            j_left_domain.push_back(std::move(j_lit));
        }
        j_left["domain"] = std::move(j_left_domain);
        j_eq["left"] = std::move(j_left);
        json::json j_right{{"var", std::to_string(right)}};
        json::json j_right_domain(json::json_type::array);
        for (const auto &[v, l] : right_domain_set)
        {
            json::json j_lit{{"lit", to_string(l)}, {"val", std::to_string(reinterpret_cast<uintptr_t>(&v))}};
            switch (value(l))
            {
            case utils::True:
                j_lit["value"] = "true";
                break;
            case utils::False:
                j_lit["value"] = "false";
                break;
            }
            j_right_domain.push_back(std::move(j_lit));
        }
        j_right["domain"] = std::move(j_right_domain);
        j_eq["right"] = std::move(j_right);
        json::json j_ctr{{"lit", to_string(ctr)}};
        switch (value(ctr))
        {
        case utils::True:
            j_ctr["value"] = "true";
            break;
        case utils::False:
            j_ctr["value"] = "false";
            break;
        }
        j_eq["ctr"] = std::move(j_ctr);
        return j_eq;
    }
} // namespace semitone
