#pragma once

#include "constr.h"

namespace semitone
{
  class sat_core;
  class theory;

  /**
   * This class is used for representing propositional clauses.
   */
  class clause final : public constr
  {
    friend class sat_core;
    friend class theory;

  private:
    clause(sat_core &s, std::vector<lit> lits);
    clause(const clause &orig) = delete;

  private:
    std::vector<lit> lits;
  };
} // namespace semitone