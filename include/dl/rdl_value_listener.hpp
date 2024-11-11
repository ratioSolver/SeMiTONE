#pragma once

#include "rdl_theory.hpp"

namespace semitone
{
  class rdl_value_listener
  {
    friend class rdl_theory;

  public:
    virtual ~rdl_value_listener()
    {
      if (th) // if the listener is still bound to a rdl theory
        th->remove_listener(*this);
    }

  protected:
    void listen_rdl(std::size_t v) noexcept
    {
      if (th->lb(v) != th->ub(v))
      { // the variable is not yet assigned
        listening.push_back(v);
        th->listening[v].insert(this);
      }
    }

  private:
    virtual void on_rdl_value_changed(std::size_t v) = 0;

  private:
    rdl_theory *th{nullptr};
    std::vector<std::size_t> listening;
  };
} // namespace semitone
