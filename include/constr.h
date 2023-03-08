#pragma once

#include "lit.h"
#include "bool.h"
#include "json.h"
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

  public:
    virtual ~constr() = default;

  protected:
    constr(sat_core &s);
    constr(const constr &orig) = delete;

  private:
    virtual constr *copy(sat_core &s) noexcept = 0;
    virtual bool propagate(const lit &p) noexcept = 0;
    virtual bool simplify() noexcept = 0;
    virtual void get_reason(const lit &p, std::vector<lit> &out_reason) const noexcept = 0;

    virtual json::json to_json() const noexcept { return json::json(); }
    friend json::json to_json(const constr &rhs) noexcept { return rhs.to_json(); }

  protected:
    std::vector<constr *> &watches(const lit &p) noexcept;
    bool enqueue(const lit &p) noexcept;

    utils::lbool value(const var &x) const noexcept;
    utils::lbool value(const lit &p) const noexcept;

    void remove_constr_from_reason(const var &x) noexcept;

  private:
    sat_core &sat;
  };
} // namespace semitone