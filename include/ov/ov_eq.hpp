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
    ov_eq(ov_theory &ov, const VARIABLE_TYPE left, const VARIABLE_TYPE right, const utils::lit &ctr) noexcept;

  private:
    [[nodiscard]] std::unique_ptr<constr> copy(sat_core &s) noexcept override;

    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    [[nodiscard]] bool simplify() noexcept override;

    [[nodiscard]] std::vector<utils::lit> get_reason(const utils::lit &p) const noexcept override;

    [[nodiscard]] json::json to_json() const noexcept override;

  private:
    ov_theory &ov;
    const VARIABLE_TYPE left;
    std::unordered_map<VARIABLE_TYPE, std::reference_wrapper<utils::enum_val>> left_domain;
    const VARIABLE_TYPE right;
    std::unordered_map<VARIABLE_TYPE, std::reference_wrapper<utils::enum_val>> right_domain;
    const utils::lit ctr;
  };
} // namespace semitone
