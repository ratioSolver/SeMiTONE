#pragma once

#include "bool.hpp"
#include "integer.hpp"
#include "rational.hpp"
#include "memory.hpp"
#include <vector>

namespace semitone
{
  class context;
  class network;

  class term
  {
  public:
    term(context &ctx, std::string_view name);
    virtual ~term() = default;

    [[nodiscard]] context &get_ctx() noexcept;
    [[nodiscard]] const std::string &get_name() const noexcept;

  private:
    context &ctx;
    const std::string name;
  };

  using expr = utils::s_ptr<term>;

  class bool_term : public term
  {
    friend class network;

  public:
    bool_term(context &ctx, std::string_view name);

    [[nodiscard]] virtual utils::lbool val() const noexcept = 0;
  };

  using bool_expr = utils::s_ptr<bool_term>;

  class bool_var final : public bool_term
  {
    friend class network;

  public:
    bool_var(context &ctx, std::string_view name);
    bool_var(context &ctx, utils::lbool val);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    utils::lbool value;
  };

  class and_expr final : public bool_term
  {
  public:
    and_expr(context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    static std::string get_name(const std::vector<bool_expr> &args);

  private:
    std::vector<bool_expr> arguments;
  };

  class or_expr final : public bool_term
  {
  public:
    or_expr(context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    static std::string get_name(const std::vector<bool_expr> &args);

  private:
    std::vector<bool_expr> arguments;
  };

  class not_expr final : public bool_term
  {
  public:
    not_expr(context &ctx, bool_expr arg);

    [[nodiscard]] bool_expr arg() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    bool_expr argument;
  };

  class int_term : public term
  {
    friend class network;

  public:
    int_term(context &ctx, std::string_view name);

    [[nodiscard]] virtual utils::integer lb() const noexcept = 0;
    [[nodiscard]] virtual utils::integer ub() const noexcept = 0;

    [[nodiscard]] virtual utils::integer val() const noexcept = 0;
  };

  using int_expr = utils::s_ptr<int_term>;

  class int_var final : public int_term
  {
    friend class network;

  public:
    int_var(context &ctx, std::string_view name);
    int_var(context &ctx, std::string_view name, const utils::integer &lb, const utils::integer &ub);
    int_var(context &ctx, const utils::integer &val);

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static utils::integer get_val(const utils::integer &lb, const utils::integer &ub);

  private:
    utils::integer value, lower_bound, upper_bound;
  };

  class int_sum final : public int_term
  {
  public:
    int_sum(context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_sub final : public int_term
  {
  public:
    int_sub(context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_mul final : public int_term
  {
  public:
    int_mul(context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_div final : public int_term
  {
  public:
    int_div(context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_lt final : public bool_term
  {
  public:
    int_lt(context &ctx, int_expr lhs, int_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    int_expr left, right;
  };

  class int_le final : public bool_term
  {
  public:
    int_le(context &ctx, int_expr lhs, int_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    int_expr left, right;
  };

  class int_eq final : public bool_term
  {
  public:
    int_eq(context &ctx, int_expr lhs, int_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    int_expr left, right;
  };

  class int_ge final : public bool_term
  {
  public:
    int_ge(context &ctx, int_expr lhs, int_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    int_expr left, right;
  };

  class int_gt final : public bool_term
  {
  public:
    int_gt(context &ctx, int_expr lhs, int_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    int_expr left, right;
  };

  class real_term : public term
  {
    friend class network;

  public:
    real_term(context &ctx, std::string_view name);

    [[nodiscard]] virtual utils::rational lb() const noexcept = 0;
    [[nodiscard]] virtual utils::rational ub() const noexcept = 0;

    [[nodiscard]] virtual utils::rational val() const noexcept = 0;
  };

  using real_expr = utils::s_ptr<real_term>;

  class real_var final : public real_term
  {
    friend class network;

  public:
    real_var(context &ctx, std::string_view name);
    real_var(context &ctx, std::string_view name, const utils::rational &lb, const utils::rational &ub);
    real_var(context &ctx, const utils::rational &val);

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static utils::rational get_val(const utils::rational &lb, const utils::rational &ub);

  private:
    utils::rational value, lower_bound, upper_bound;
  };

  class real_sum final : public real_term
  {
  public:
    real_sum(context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_sub final : public real_term
  {
  public:
    real_sub(context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_mul final : public real_term
  {
  public:
    real_mul(context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_div final : public real_term
  {
  public:
    real_div(context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_lt final : public bool_term
  {
  public:
    real_lt(context &ctx, real_expr lhs, real_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    real_expr left, right;
  };

  class real_le final : public bool_term
  {
  public:
    real_le(context &ctx, real_expr lhs, real_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    real_expr left, right;
  };

  class real_eq final : public bool_term
  {
  public:
    real_eq(context &ctx, real_expr lhs, real_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    real_expr left, right;
  };

  class real_ge final : public bool_term
  {
  public:
    real_ge(context &ctx, real_expr lhs, real_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    real_expr left, right;
  };

  class real_gt final : public bool_term
  {
  public:
    real_gt(context &ctx, real_expr lhs, real_expr rhs);

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    real_expr left, right;
  };

  class string_var : public term
  {
  public:
    string_var(context &ctx, std::string val);

    [[nodiscard]] std::string val() const noexcept;
  };

  using string_expr = utils::s_ptr<string_var>;
} // namespace semitone
