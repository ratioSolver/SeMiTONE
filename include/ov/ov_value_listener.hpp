#pragma once

#include "ov_theory.hpp"
#include "sat_value_listener.hpp"

namespace semitone
{
  class ov_value_listener : private sat_value_listener
  {
    friend class ov_theory;

  public:
    virtual ~ov_value_listener()
    {
      if (th) // if the listener is still bound to a ov theory
        th->remove_listener(*this);
    }

  protected:
    void listen_ov(VARIABLE_TYPE v) noexcept
    {
      listening.push_back(v);
      for (auto &d : th->domains[v])
        if (th->get_sat().value(d.second) == utils::Undefined)
          listening_map[variable(d.second)].push_back(v);
    }

  private:
    virtual void ov_value_change(VARIABLE_TYPE v) = 0;

    void on_sat_value_changed(VARIABLE_TYPE v) override
    {
      for (auto &d : listening_map.at(v))
        ov_value_change(d);
    }

  private:
    ov_theory *th{nullptr};
    std::vector<VARIABLE_TYPE> listening;
    std::unordered_map<VARIABLE_TYPE, std::vector<VARIABLE_TYPE>> listening_map;
  };
} // namespace semitone
