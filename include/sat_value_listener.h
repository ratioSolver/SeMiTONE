
#pragma once

#include "sat_core.h"
#include <algorithm>

namespace semitone
{
  using sat_ptr = utils::c_ptr<sat_core>;

  class sat_value_listener
  {
    friend class sat_core;
    friend class sat_stack;

  public:
    sat_value_listener(sat_ptr s) : sat(s) { sat->listeners.push_back(this); }
    sat_value_listener(const sat_value_listener &that) = delete;
    virtual ~sat_value_listener() { sat->listeners.erase(std::find(sat->listeners.cbegin(), sat->listeners.cend(), this)); }

  protected:
    void listen_sat(var v) noexcept { sat->listen(v, *this); }

  private:
    virtual void sat_value_change(const var &) {}

  private:
    sat_ptr sat;
  };
} // namespace semitone