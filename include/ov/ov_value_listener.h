#pragma once

#include "ov_theory.h"

namespace semitone
{
  class ov_value_listener
  {
    friend class ov_theory;

  public:
    ov_value_listener(ov_theory &s) : th(s) {}
    ov_value_listener(const ov_value_listener &that) = delete;
    virtual ~ov_value_listener() = default;

  protected:
    inline void listen_set(var v) noexcept { th.listen(v, this); }

  private:
    virtual void ov_value_change(const var &) {}

  private:
    ov_theory &th;
  };
} // namespace semitone