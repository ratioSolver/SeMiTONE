#pragma once

#include "sat_core.hpp"
#include <algorithm>

namespace semitone
{
  class sat_value_listener
  {
    friend class sat_core;

  public:
    sat_value_listener(std::shared_ptr<sat_core> sat) noexcept : sat(sat) {}
    virtual ~sat_value_listener() = default;

  protected:
    void listen_sat(VARIABLE_TYPE v) noexcept { sat->listen(v, *this); }

  private:
    virtual void on_sat_value_changed(VARIABLE_TYPE v) = 0;

  private:
    std::shared_ptr<sat_core> sat;
  };
} // namespace semitone
