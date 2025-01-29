#pragma once

#include "context.hpp"
#include <unordered_map>

namespace semitone
{
  class solver
  {
  public:
    solver(context &ctx);

    void add(bool_expr expr);

  private:
    bool_expr to_cnf(bool_expr expr);
    bool_expr push_negations(bool_expr expr);
    bool_expr distribute(bool_expr expr);

    size_t add_var(std::string_view name);
    void add_clause(bool_expr expr);

  private:
    context &ctx;
    std::unordered_map<std::string, size_t> var_map;
  };
} // namespace semitone
