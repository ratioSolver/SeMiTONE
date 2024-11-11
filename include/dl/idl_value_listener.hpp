#pragma once

#include "idl_theory.hpp"

namespace semitone
{
  class idl_value_listener
  {
    friend class idl_theory;

  public:
    virtual ~idl_value_listener()
    {
      if (th) // if the listener is still bound to a idl theory
        th->remove_listener(*this);
    }

  protected:
    void listen_idl(utils::var v) noexcept
    {
      if (th->lb(v) != th->ub(v))
      { // the variable is not yet assigned
        listening.push_back(v);
        th->listening[v].insert(this);
      }
    }

  private:
    virtual void on_idl_value_changed(utils::var v) = 0;

  private:
    idl_theory *th{nullptr};
    std::vector<utils::var> listening;
  };
} // namespace semitone
