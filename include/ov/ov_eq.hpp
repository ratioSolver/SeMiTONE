#pragma once

#include <unordered_map>
#include "constr.hpp"
#include "enum.hpp"

namespace semitone
{
  class ov_theory;

  class ov_eq final : public constr
  {
  public:
    ov_eq(ov_theory &ov, const VARIABLE_TYPE left, const VARIABLE_TYPE right, const lit &ctr);

  private:
    std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    bool propagate(const lit &p) noexcept override;
    bool simplify() noexcept override;

    std::vector<lit> get_reason(const lit &p) const noexcept override;

    json::json to_json() const noexcept override;

  private:
    ov_theory &ov;
    const VARIABLE_TYPE left;
    std::unordered_map<VARIABLE_TYPE, std::reference_wrapper<utils::enum_val>> left_domain;
    std::unordered_map<utils::enum_val *, lit> left_domain_set;
    const VARIABLE_TYPE right;
    std::unordered_map<VARIABLE_TYPE, std::reference_wrapper<utils::enum_val>> right_domain;
    std::unordered_map<utils::enum_val *, lit> right_domain_set;
    const lit ctr;
  };
} // namespace semitone
