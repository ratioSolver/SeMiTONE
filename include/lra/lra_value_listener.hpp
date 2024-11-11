#pragma once

#include "lra_theory.hpp"

namespace semitone
{
  class lra_value_listener
  {
    friend class lra_theory;

  public:
    virtual ~lra_value_listener()
    {
      if (th) // if the listener is still bound to a lra theory
        th->remove_listener(*this);
    }

  protected:
    void listen_lra(std::size_t v) noexcept
    {
      if (th->lb(v) != th->ub(v))
      { // the variable is not yet assigned
        listening.push_back(v);
        th->listening[v].insert(this);
      }
    }

  private:
    virtual void on_lra_value_changed(std::size_t v) = 0;

  private:
    lra_theory *th{nullptr};
    std::vector<std::size_t> listening;
  };
} // namespace semitone