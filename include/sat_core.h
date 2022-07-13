#pragma once

#include "semitone_export.h"
#include "lit.h"
#include <vector>

namespace semitone
{
  class sat_core
  {
  public:
    SEMITONE_EXPORT sat_core();
    SEMITONE_EXPORT ~sat_core();

    SEMITONE_EXPORT var new_var() noexcept; // creates a new propositional variable..

  private:
    std::vector<lbool> assigns; // the current assignments..
    std::vector<size_t> level;  // for each variable, the decision level it was assigned..
  };
} // namespace semitone
