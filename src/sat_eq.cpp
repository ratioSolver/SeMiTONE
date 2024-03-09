#include <cassert>
#include "sat_eq.hpp"
#include "sat_core.hpp"

namespace semitone
{
    sat_eq::sat_eq(sat_core &s, const utils::lit &l, const utils::lit &r, const utils::lit &ctr) noexcept : constr(s), left(l), right(r), ctr(ctr)
    {
        assert(s.root_level());
        assert(value(l) == utils::Undefined);
        assert(value(r) == utils::Undefined);
        assert(value(ctr) == utils::Undefined);
        watches(l).emplace_back(*this);
        watches(!l).emplace_back(*this);
        watches(r).emplace_back(*this);
        watches(!r).emplace_back(*this);
        watches(ctr).emplace_back(*this);
        watches(!ctr).emplace_back(*this);
    }

    std::unique_ptr<constr> sat_eq::copy(sat_core &s) noexcept { return std::make_unique<sat_eq>(s, left, right, ctr); }

    bool sat_eq::propagate(const utils::lit &p) noexcept
    {
        assert(value(p) == utils::True);
        watches(p).emplace_back(*this);
        if (p == left)
        {
            switch (value(ctr))
            {
            case utils::True:
                return value(variable(p)) == utils::True ? enqueue(right) : enqueue(!right);
            case utils::False:
                return value(variable(p)) == utils::True ? enqueue(!right) : enqueue(right);
            default:
                switch (value(right))
                {
                case utils::True:
                    return value(variable(p)) == utils::True ? enqueue(ctr) : enqueue(!ctr);
                case utils::False:
                    return value(variable(p)) == utils::True ? enqueue(!ctr) : enqueue(ctr);
                default:
                    return true;
                }
            }
        }
        if (p == right)
        {
            switch (value(ctr))
            {
            case utils::True:
                return value(variable(p)) == utils::True ? enqueue(left) : enqueue(!left);
            case utils::False:
                return value(variable(p)) == utils::True ? enqueue(!left) : enqueue(left);
            default:
                switch (value(left))
                {
                case utils::True:
                    return value(variable(p)) == utils::True ? enqueue(ctr) : enqueue(!ctr);
                case utils::False:
                    return value(variable(p)) == utils::True ? enqueue(!ctr) : enqueue(ctr);
                default:
                    return true;
                }
            }
        }
        assert(p == ctr);
        switch (value(left))
        {
        case utils::True:
            return value(variable(p)) == utils::True ? enqueue(right) : enqueue(!right);
        case utils::False:
            return value(variable(p)) == utils::True ? enqueue(!right) : enqueue(right);
        default:
            switch (value(right))
            {
            case utils::True:
                return value(variable(p)) == utils::True ? enqueue(left) : enqueue(!left);
            case utils::False:
                return value(variable(p)) == utils::True ? enqueue(!left) : enqueue(left);
            default:
                return true;
            }
        }
        return true;
    }

    bool sat_eq::simplify() noexcept { return value(left) != utils::Undefined && value(right) != utils::Undefined && value(ctr) != utils::Undefined; }

    std::vector<utils::lit> sat_eq::get_reason(const utils::lit &p) const noexcept
    {
        assert(value(left) == value(right) || value(ctr) == utils::False);
        assert(value(left) != value(right) || value(ctr) == utils::True);
        if (is_undefined(p))
            return {!left, !right, !ctr};
        else if (p == left)
            return {!right, !ctr};
        else if (p == right)
            return {!left, !ctr};
        else
        {
            assert(p == ctr);
            return {!left, !right};
        }
    }

    json::json sat_eq::to_json() const noexcept
    {
        json::json j_eq;
        json::json j_left{{"lit", to_string(left)}};
        switch (value(left))
        {
        case utils::True:
            j_left["value"] = "true";
            break;
        case utils::False:
            j_left["value"] = "false";
            break;
        }
        j_eq["left"] = std::move(j_left);
        json::json j_right{{"lit", to_string(right)}};
        switch (value(right))
        {
        case utils::True:
            j_right["value"] = "true";
            break;
        case utils::False:
            j_right["value"] = "false";
            break;
        }
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
