#pragma once

#include "bool.hpp"
#include "integer.hpp"
#include "rational.hpp"
#include "memory.hpp"
#include <vector>

namespace semitone
{
  class context;

  class var
  {
  public:
    var(const context &ctx, std::string_view name);
    virtual ~var() = default;

    [[nodiscard]] const context &get_ctx() const noexcept;
    [[nodiscard]] const std::string &get_name() const noexcept;

  private:
    const context &ctx;
    const std::string name;
  };

  using expr = utils::s_ptr<var>;

  class bool_var : public var
  {
  public:
    bool_var(const context &ctx, std::string_view name);
    bool_var(const context &ctx, utils::lbool val);
    virtual ~bool_var() = default;

    [[nodiscard]] virtual utils::lbool val() const noexcept;

  private:
    utils::lbool value;
  };

  using bool_expr = utils::s_ptr<bool_var>;

  class and_expr final : public bool_var
  {
  public:
    and_expr(const context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    static std::string get_name(const std::vector<bool_expr> &args);

  private:
    std::vector<bool_expr> arguments;
  };

  class or_expr final : public bool_var
  {
  public:
    or_expr(const context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    static std::string get_name(const std::vector<bool_expr> &args);

  private:
    std::vector<bool_expr> arguments;
  };

  class not_expr final : public bool_var
  {
  public:
    not_expr(const context &ctx, bool_expr arg);

    [[nodiscard]] bool_expr arg() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    bool_expr argument;
  };

  class int_var : public var
  {
  public:
    int_var(const context &ctx, std::string_view name);
    int_var(const context &ctx, std::string_view name, const utils::integer &lb, const utils::integer &ub);
    int_var(const context &ctx, const utils::integer &val);
    virtual ~int_var() = default;

    [[nodiscard]] virtual utils::integer lb() const noexcept;
    [[nodiscard]] virtual utils::integer ub() const noexcept;

    [[nodiscard]] virtual utils::integer val() const noexcept;

  private:
    static utils::integer get_val(const utils::integer &lb, const utils::integer &ub);

  private:
    utils::integer value, lower_bound, upper_bound;
  };

  using int_expr = utils::s_ptr<int_var>;

  class int_sum final : public int_var
  {
  public:
    int_sum(const context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_sub final : public int_var
  {
  public:
    int_sub(const context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_mul final : public int_var
  {
  public:
    int_mul(const context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class int_div final : public int_var
  {
  public:
    int_div(const context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    static std::string get_name(const std::vector<int_expr> &args);

  private:
    std::vector<int_expr> arguments;
  };

  class real_var : public var
  {
  public:
    real_var(const context &ctx, std::string_view name);
    real_var(const context &ctx, std::string_view name, const utils::rational &lb, const utils::rational &ub);
    real_var(const context &ctx, const utils::rational &val);
    virtual ~real_var() = default;

    [[nodiscard]] virtual utils::rational lb() const noexcept;
    [[nodiscard]] virtual utils::rational ub() const noexcept;

    [[nodiscard]] virtual utils::rational val() const noexcept;

  private:
    static utils::rational get_val(const utils::rational &lb, const utils::rational &ub);

  private:
    utils::rational value, lower_bound, upper_bound;
  };

  using real_expr = utils::s_ptr<real_var>;

  class real_sum final : public real_var
  {
  public:
    real_sum(const context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_sub final : public real_var
  {
  public:
    real_sub(const context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_mul final : public real_var
  {
  public:
    real_mul(const context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class real_div final : public real_var
  {
  public:
    real_div(const context &ctx, std::vector<real_expr> &&args);

    [[nodiscard]] const std::vector<real_expr> &args() const noexcept;

    [[nodiscard]] utils::rational lb() const noexcept override;
    [[nodiscard]] utils::rational ub() const noexcept override;

    [[nodiscard]] utils::rational val() const noexcept override;

  private:
    static std::string get_name(const std::vector<real_expr> &args);

  private:
    std::vector<real_expr> arguments;
  };

  class string_var : public var
  {
  public:
    string_var(const context &ctx, std::string val);
    virtual ~string_var() = default;

    [[nodiscard]] std::string val() const noexcept;
  };

  using string_expr = utils::s_ptr<string_var>;
} // namespace semitone
