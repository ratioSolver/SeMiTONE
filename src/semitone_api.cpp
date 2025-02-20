#include "semitone_api.hpp"
#include "sat_core.hpp"
#include "lra_theory.hpp"

namespace semitone
{
    [[nodiscard]] json::json to_json(const constr &rhs) noexcept { return rhs.to_json(); }

    [[nodiscard]] json::json to_json(const lra_theory &rhs) noexcept
    {
        json::json j_th;

        json::json j_vars(json::json_type::array);
        for (size_t i = 0; i < rhs.vals.size(); ++i)
        {
            json::json var;
            var["name"] = std::to_string(i).c_str();
            var["value"] = to_string(rhs.value(i)).c_str();
            if (!is_negative_infinite(rhs.lb(i)))
                var["lb"] = to_string(rhs.lb(i)).c_str();
            if (!is_positive_infinite(rhs.ub(i)))
                var["ub"] = to_string(rhs.ub(i)).c_str();
            j_vars.push_back(std::move(var));
        }
        j_th["vars"] = std::move(j_vars);

        json::json j_asrts(json::json_type::array);
        for (const auto &c_asrts : rhs.v_asrts)
        {
            json::json j_asrt;
            j_asrt["lit"] = to_string(c_asrts.second->b).c_str();
            switch (rhs.get_sat().value(c_asrts.second->b))
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
            j_asrt["constr"] = ("x" + std::to_string(c_asrts.first) + (c_asrts.second->o == geq ? " >= " : " <= ") + to_string(c_asrts.second->v)).c_str();
            j_asrts.push_back(std::move(j_asrt));
        }
        j_th["asrts"] = std::move(j_asrts);

        json::json j_tabl(json::json_type::array);
        for (auto it = rhs.tableau.cbegin(); it != rhs.tableau.cend(); ++it)
        {
            json::json j_row;
            j_row["var"] = ("x" + std::to_string(it->first)).c_str();
            j_row["expr"] = to_string(it->second->l).c_str();
        }
        j_th["tableau"] = std::move(j_tabl);

        return j_th;
    }
} // namespace semitone
