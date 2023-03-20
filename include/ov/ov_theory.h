#pragma once

#include "sat_core.h"
#include "theory.h"
#include "enum.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <set>

namespace semitone
{
  class ov_value_listener;

  class ov_theory final : public theory
  {
    friend class ov_value_listener;

  public:
    SEMITONE_EXPORT ov_theory(sat_ptr sat);
    ov_theory(const ov_theory &orig) = delete;

    SEMITONE_EXPORT var new_var(const std::vector<utils::enum_val *> &items, const bool enforce_exct_one = true) noexcept; // creates and returns a new object variable having the given domain..
    SEMITONE_EXPORT var new_var(const std::vector<lit> &lits, const std::vector<utils::enum_val *> &vals) noexcept;        // creates and returns a new object variable having the given domain, the presence of the values into the domain is controlled by the `lits` literals..

    SEMITONE_EXPORT lit allows(const var &v, utils::enum_val &val) const noexcept; // returns the literal controlling the presence of the `val` value into the domain of variable `v`..
    SEMITONE_EXPORT lit new_eq(const var &left, const var &right) noexcept;        // creates an equality constraints between `left` and `right` variables returning the literal that controls it..

    SEMITONE_EXPORT std::unordered_set<utils::enum_val *> value(var v) const noexcept; // returns the current domain of the object variable `v`..

  private:
    bool propagate(const lit &) noexcept override { return true; }
    bool check() noexcept override { return true; }
    void push() noexcept override {}
    void pop() noexcept override {}

    SEMITONE_EXPORT void listen(const var &v, ov_value_listener *const l) noexcept;

  private:
    std::vector<std::unordered_map<utils::enum_val *, lit>> assigns; // the current assignments (val to literal)..
    std::unordered_map<std::string, lit> exprs;                      // the already existing expressions (string to literal)..
    std::unordered_map<var, std::set<var>> is_contained_in;          // the propositional variable contained in the object variables (bool variable to object variables)..
  };
} // namespace semitone