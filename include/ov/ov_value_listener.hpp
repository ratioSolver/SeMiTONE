#pragma once

#include "ov_theory.hpp"
#include "sat_value_listener.hpp"

namespace semitone
{
  class ov_value_listener : protected sat_value_listener
  {
    friend class ov_theory;

  public:
    virtual ~ov_value_listener()
    {
      if (th) // if the listener is still bound to a ov theory
        th->remove_listener(*this);
    }

  protected:
    void listen_ov(utils::var v) noexcept
    {
      listening.push_back(v);
      for (auto &d : th->domains[v])
        if (th->get_sat().value(d.second) == utils::Undefined)
          listening_map[variable(d.second)].push_back(v);
    }

  private:
    virtual void on_ov_value_changed(utils::var v) = 0;

    void on_sat_value_changed(utils::var v) override
    {
      for (auto &d : listening_map.at(v))
        on_ov_value_changed(d);
    }

  private:
    ov_theory *th{nullptr};
    std::vector<utils::var> listening;
    std::unordered_map<utils::var, std::vector<utils::var>> listening_map;
  };
} // namespace semitone
