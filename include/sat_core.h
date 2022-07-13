#pragma once

#include "semitone_export.h"
#include "lit.h"
#include <vector>
#include <nlohmann/json.hpp>

namespace semitone
{
  class constr;

  class sat_core
  {
    friend class constr;

  public:
    SEMITONE_EXPORT sat_core();
    SEMITONE_EXPORT ~sat_core();

    SEMITONE_EXPORT var new_var() noexcept; // creates a new propositional variable..

    inline lbool value(const var &x) const noexcept { return assigns.at(x); } // returns the value of variable 'x'..
    inline lbool value(const lit &p) const noexcept
    {
      switch (value(variable(p)))
      {
      case True:
        return sign(p) ? True : False;
      case False:
        return sign(p) ? False : True;
      default:
        return Undefined;
      }
    }

    nlohmann::json to_json() const noexcept;

  private:
    std::vector<lbool> assigns;                 // the current assignments..
    std::vector<size_t> level;                  // for each variable, the decision level it was assigned..
    std::vector<constr *> constrs;              // the collection of problem constraints..
    std::vector<std::vector<constr *>> watches; // for each literal 'p', a list of constraints watching 'p'..
  };
} // namespace semitone
