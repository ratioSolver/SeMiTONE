#pragma once

#include "sat_core.hpp"

namespace semitone
{
  /**
   * @class sat_value_listener
   * @brief Interface for listening to changes in the value of variables in a SAT solver.
   *
   * The `sat_value_listener` class provides an interface for classes that want to be notified
   * when the value of a variable in a SAT solver changes. Classes that implement this interface
   * can register themselves as listeners with a SAT solver and receive callbacks when the value
   * of the registered variables changes.
   */
  class sat_value_listener
  {
    friend class sat_core;

  public:
    virtual ~sat_value_listener()
    {
      if (sat) // if the listener is still bound to a sat core
        sat->remove_listener(*this);
    }

  protected:
    void listen_sat(VARIABLE_TYPE v) noexcept
    {
      if (sat->value(v) == utils::Undefined)
      { // the variable is not yet assigned
        listening.push_back(v);
        sat->listening[v].insert(this);
      }
    }

  private:
    virtual void on_sat_value_changed(VARIABLE_TYPE v) = 0;

  private:
    sat_core *sat{nullptr};
    std::vector<VARIABLE_TYPE> listening;
  };
} // namespace semitone
