#pragma once

#include "ov_theory.h"
#include "sat_value_listener.h"

namespace semitone
{
  class ov_value_listener : public sat_value_listener
  {
    friend class ov_theory;

  public:
    ov_value_listener(ov_theory &s) : sat_value_listener(s.sat), th(s) {}
    ov_value_listener(const ov_value_listener &that) = delete;
    virtual ~ov_value_listener() = default;

  protected:
    inline void listen_set(var v) noexcept { th.listen(v, this); }

  private:
    virtual void ov_value_change(const var &) {}

    void sat_value_change(const var &v) override
    {
      for (auto l : th.is_contained_in[v])
        ov_value_change(l);
    }

  private:
    ov_theory &th;
  };
} // namespace semitone