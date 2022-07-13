#include "clause.h"
#include <cassert>

namespace semitone
{
    clause::clause(sat_core &s, std::vector<lit> lits) : constr(s), lits(std::move(lits)) {}

    nlohmann::json clause::to_json() const noexcept
    {
        nlohmann::json j_cl;

        nlohmann::json::array_t j_lits;
        for (const auto &l : lits)
        {
            nlohmann::json j_lit;
            j_lit["lit"] = to_string(l);
            switch (value(l))
            {
            case True:
                j_lit["val"] = "T";
                break;
            case False:
                j_lit["val"] = "F";
                break;
            case Undefined:
                j_lit["val"] = "U";
                break;
            }
            j_lits.push_back(j_lit);
        }
        j_cl["lits"] = j_lits;

        return j_cl;
    }
} // namespace smt