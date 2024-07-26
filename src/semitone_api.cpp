#include "semitone_api.hpp"
#include "sat_core.hpp"
#include "lra_theory.hpp"
#include "lra_assertion.hpp"

namespace semitone
{
    [[nodiscard]] json::json to_json(const constr &rhs) noexcept { return rhs.to_json(); }

    [[nodiscard]] json::json to_json(const lra_assertion &rhs) noexcept
    {
        json::json j_asrt;
        j_asrt["lit"] = to_string(rhs.b);
        switch (rhs.th.get_sat().value(rhs.b))
        {
        case utils::True:
            j_asrt["val"] = "T";
            break;
        case utils::False:
            j_asrt["val"] = "F";
            break;
        case utils::Undefined:
            j_asrt["val"] = "U";
            break;
        }
        j_asrt["constr"] = "x" + std::to_string(rhs.x) + (rhs.o == geq ? " >= " : " <= ") + to_string(rhs.v);
        return j_asrt;
    }

    [[nodiscard]] json::json to_json(const lra_eq &rhs) noexcept
    {
        json::json j_row;
        j_row["var"] = "x" + std::to_string(rhs.x);
        j_row["expr"] = to_string(rhs.l);
        return j_row;
    }

    [[nodiscard]] json::json to_json(const lra_theory &rhs) noexcept
    {
        json::json j_th;

        json::json j_vars(json::json_type::array);
        for (size_t i = 0; i < rhs.vals.size(); ++i)
        {
            json::json var;
            var["name"] = std::to_string(i);
            var["value"] = to_string(rhs.value(i));
            if (!is_negative_infinite(rhs.lb(i)))
                var["lb"] = to_string(rhs.lb(i));
            if (!is_positive_infinite(rhs.ub(i)))
                var["ub"] = to_string(rhs.ub(i));
            j_vars.push_back(std::move(var));
        }
        j_th["vars"] = std::move(j_vars);

        json::json j_asrts(json::json_type::array);
        for (const auto &c_asrts : rhs.v_asrts)
            j_asrts.push_back(to_json(*c_asrts.second));
        j_th["asrts"] = std::move(j_asrts);

        json::json j_tabl(json::json_type::array);
        for (auto it = rhs.tableau.cbegin(); it != rhs.tableau.cend(); ++it)
            j_tabl.push_back(to_json(*it->second));
        j_th["tableau"] = std::move(j_tabl);

        return j_th;
    }
} // namespace semitone
