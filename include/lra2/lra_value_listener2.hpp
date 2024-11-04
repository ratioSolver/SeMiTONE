#pragma once

#include "lra_theory2.hpp"

namespace semitone
{
  class lra_value_listener2
  {
    friend class lra_theory2;

  public:
    virtual ~lra_value_listener2()
    {
      if (th) // if the listener is still bound to a lra theory
        th->remove_listener(*this);
    }

  protected:
    void listen_lra(VARIABLE_TYPE v) noexcept
    {
      if (th->lb(v) != th->ub(v))
      { // the variable is not yet assigned
        listening.push_back(v);
        th->listening[v].insert(this);
      }
    }

  private:
    virtual void on_lra_value_changed(VARIABLE_TYPE v) = 0;

  private:
    lra_theory2 *th{nullptr};
    std::vector<VARIABLE_TYPE> listening;
  };
} // namespace semitone