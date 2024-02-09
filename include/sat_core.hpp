#pragma once

#include <memory>
#include "constr.hpp"

namespace semitone
{
  class sat_core
  {
    friend class constr;

  public:
    /**
     * @brief Construct a new sat core object.
     *
     */
    sat_core();

    /**
     * @brief Return the value of a variable.
     *
     * @param x The variable.
     * @return The value of the variable.
     */
    inline utils::lbool value(const VARIABLE_TYPE &x) const noexcept { return assigns.at(x); }
    /**
     * @brief Return the value of a literal.
     *
     * @param p The literal.
     * @return The value of the literal.
     */
    inline utils::lbool value(const lit &p) const noexcept
    {
      switch (value(variable(p)))
      {
      case utils::True:
        return sign(p) ? utils::True : utils::False;
      case utils::False:
        return sign(p) ? utils::False : utils::True;
      default:
        return utils::Undefined;
      }
    }

  private:
    std::vector<std::unique_ptr<constr>> constrs;                     // the collection of problem constraints..
    std::vector<std::vector<std::reference_wrapper<constr>>> watches; // for each literal `p`, a list of constraints watching `p`..
    std::vector<utils::lbool> assigns;                                // for each variable, the current assignment..
    std::vector<constr *> reason;                                     // for each variable, the constraint that implied its value..
    std::vector<size_t> level;                                        // for each variable, the decision level it was assigned..
  };
} // namespace semitone
