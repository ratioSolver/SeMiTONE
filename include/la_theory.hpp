#pragma once

#include "theory.hpp"
#include "inf_rational.hpp"
#include <unordered_map>

namespace semitone
{
  class la_theory : public theory
  {
  public:
    la_theory(network &slv) noexcept;

    void push() noexcept override;
    void pop() noexcept override;

  private:
    [[nodiscard]] size_t add_real(std::string_view name, const utils::inf_rational &lb = utils::inf_rational(utils::rational::negative_infinite), const utils::inf_rational &ub = utils::inf_rational(utils::rational::positive_infinite));

  private:
    std::unordered_map<std::string, size_t> var_map;
    /**
     * Represents the bound of a variable and the reason for its existence.
     */
    struct bound
    {
      utils::inf_rational value;      // the value of the bound..
      std::vector<utils::lit> reason; // the reason for the value..
    };

    std::vector<bound> c_bounds;           // the current bounds..
    std::vector<utils::inf_rational> vals; // the current values..
  };
} // namespace semitone
