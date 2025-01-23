#include "var.hpp"

namespace semitone
{
  class context
  {
  public:
    context(const context &) = delete;
    context(context &&) = default;

    bool_expr bool_var(std::string_view name);
    bool_expr bool_const(bool value);

    bool_expr mk_and(std::vector<bool_expr> &&args);
  };
} // namespace semitone
