#pragma once

#include "json.hpp"

namespace semitone
{
  class constr;
  class lra_theory;

  /**
   * @brief Converts a `constr` object to a JSON representation.
   *
   * This function takes a `constr` object as input and converts it to a JSON object.
   * The resulting JSON object represents the `constr` object in a serialized format.
   *
   * @param rhs The `constr` object to be converted to JSON.
   * @return A JSON object representing the `constr` object.
   */
  [[nodiscard]] json::json to_json(const constr &rhs) noexcept;

  /**
   * @brief Converts an instance of the lra_theory class to a JSON object.
   *
   * @param rhs The lra_theory object to convert.
   * @return A JSON object representing the lra_theory object.
   */
  [[nodiscard]] json::json to_json(const lra_theory &rhs) noexcept;
} // namespace semitone
