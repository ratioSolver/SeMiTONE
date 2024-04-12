#pragma once

#include "sat_core.hpp"
#include <algorithm>

namespace semitone
{
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
      listening.push_back(v);
      if (sat->value(v) == utils::Undefined)
        sat->listening[v].insert(this);
    }

  private:
    virtual void on_sat_value_changed(VARIABLE_TYPE v) = 0;

  private:
    sat_core *sat{nullptr};
    std::vector<VARIABLE_TYPE> listening;
  };
} // namespace semitone
