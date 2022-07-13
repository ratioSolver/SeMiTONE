#pragma once

#include "lit.h"
#include <vector>
#include <nlohmann/json.hpp>

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

  protected:
    std::vector<constr *> &watches(const lit &p) noexcept;

    lbool value(const var &x) const noexcept;
    lbool value(const lit &p) const noexcept;

    virtual nlohmann::json to_json() const noexcept { return {}; }

  private:
    sat_core &sat;
    size_t id;
  };
} // namespace semitone