#pragma once

#include "lit.h"
#include <vector>

namespace semitone
{
  class sat_core;

  /**
   * This class is used for representing propositional constraints.
   */
  class constr
  {
    friend class sat_core;

  protected:
    constr(sat_core &s);
    constr(const constr &orig) = delete;
    virtual ~constr() = default;

  private:
    sat_core &sat;
    size_t id;
  };
} // namespace semitone